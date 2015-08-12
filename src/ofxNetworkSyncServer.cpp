//
//  ofxNetworkSyncServer.cpp
//  networkSyncVideoPlayer
//
//  Created by FURUDATE Ken on 2/15/15.
//
//

#include "ofxNetworkSyncServer.h"

ofxNetworkSyncServer::ofxNetworkSyncServer() : tcpServer(NULL){
	
}
ofxNetworkSyncServer::~ofxNetworkSyncServer(){
	close();
}

bool ofxNetworkSyncServer::setup(int tcpPort, bool _bAutoCalibration, int _finderRecvPort, int _finderSendPort){
	
	bAutoCalibration = _bAutoCalibration;
	clientStates.clear();
	//tcpLastConnectionID = -1;
	finderRecvPort = _finderRecvPort;
	finderSendPort = _finderSendPort;
	
	tcpServer = new ofxTCPServer();
	if(tcpServer->setup(tcpPort, false)){
		ofLogVerbose("ofxNetworkSyncServer") << "Server is waiting on port: " << tcpPort;
		if(finderResponder.Create()){
			if(finderResponder.Bind(finderRecvPort)){
				finderResponder.SetNonBlocking(true);
				ofLogVerbose("ofxNetworkSyncServer") << "Finder Responder is waiting on port: " << finderRecvPort;
				startThread(true);
			}else{
				ofLogError("ofxNetworkSyncServer") << "failed to create Finder Responder on port: " << finderRecvPort;
				return false;
			}
		}
		return true;
	}else{
		ofLogError("ofxNetworkSyncServer") << "Failed to start server on port: " << tcpPort;
		return false;
	}
}
void ofxNetworkSyncServer::close(){
	if(isThreadRunning()){
		stopThread();
		waitForThread();
	}
	finderResponder.Close();
//	ofLogVerbose("ofxNetworkSyncServer") << "close server... bye";
	try{
		if(tcpServer != NULL){
			for (auto & state : clientStates) {
				state->close();
				delete state;
			}
			clientStates.clear();
			tcpServer->close();
//			ofSleepMillis(1000);
//			delete tcpServer;
		}
	}catch(const Poco::Exception& exc){
//		ofLogError() << exc.displayText() << endl;
	}
	//tcpLastConnectionID = -1;
}
void ofxNetworkSyncServer::update(){
	if(tcpServer == NULL || ! tcpServer->isConnected() || tcpServer->getLastID() <= 0){
		// no connection
		return;
	}
	
	
	// connection check
	for(int i=0; i<tcpServer->getLastID(); i++){
		if(tcpServer->isClientConnected(i)){
			bool bAlreadyConnected = false;
			for(auto & s : clientStates){
				if(s->getClientID() == i){
					bAlreadyConnected = true;
					break;
				}
			}
			if(! bAlreadyConnected){
				ofLogVerbose("ofxNetworkSyncServer") << "New Connection established: #"<< i << " " << tcpServer->getClientIP(i) << ":" << tcpServer->getClientPort(i);
				// new connection
				clientStates.push_back(new ofxNetworkSyncClientState(this, tcpServer, i));
				ofAddListener(clientStates.back()->clientDisconnected, this, &ofxNetworkSyncServer::onClientDisconnected);
				ofNotifyEvent(newClientConnected, i, this);
				// New Client will be calibrated automatically.
				if(bAutoCalibration){
					ofLogVerbose("ofxNetworkSyncServer") << "Client#" << i << " : Start Calibration";
					clientStates.back()->startCalibration();
				}
			}
		}
	}
	
	
//	if(tcpLastConnectionID != tcpServer->getLastID()){
//		int clientId = tcpServer->getLastID()-1;
//		ofLogVerbose("ofxNetworkSyncServer") << "New Connection established: #"<< clientId << " " << tcpServer->getClientIP(clientId) << ":" << tcpServer->getClientPort(clientId);
//		// new connection
//		clientStates.push_back(new ofxNetworkSyncClientState(this, tcpServer, clientId));
//		ofNotifyEvent(newClientConnected, clientId, this);
//		
//		tcpLastConnectionID = tcpServer->getLastID();
//		
//		// New Client will be calibrated automatically.
//		if(bAutoCalibration){
//			ofLogVerbose("ofxNetworkSyncServer") << "Client#" << clientId << " : Start Calibration";
//			clientStates.back()->startCalibration();
//		}
//		//			ofNotifyEvent(newClientComming, clientStates.back(), this);
//	}
}

void ofxNetworkSyncServer::drawStatus(int x, int y){
	ostringstream ostr("");
	
	if(tcpServer != NULL && tcpServer->isConnected()){
		ostr << "Server is waiting on port:" << tcpServer->getPort() << endl;
	}else{
		ostr << "failed to start server" << endl;
	}
	
	
	if(clientStates.size() > 0){
		ostr << "Clients: " << endl;
		for(auto & c : clientStates){
			ostr << "    #" << c->getClientID() << " " << c->getIpAddr() << ":" << c->getPort() << " : ";
			if(! c->isConnected()){
				ostr << "NOT CONNECTED";
			}else if(c->isCalibrating()){
				ostr << "CALIBRATING";
			}else if(c->isCalibrated()){
				ostr << "CALIBRATED" << endl;
			}else{
				ostr << "NOT CALIBRATED";
			}
			ostr << endl;
		}
	}else{
		ostr << "Here is no clients." << endl;
	}
	
	ostr << "now:" << getSyncedElapsedTimeMillis();
	
	ofSetColor(255);
	ofDrawBitmapString(ostr.str(), x, y);
}

void ofxNetworkSyncServer::onClientMessageReceived(int clientId, string message){
	ofxNetworkSyncServerMessage m = {clientId, message};
	ofNotifyEvent(messageReceived, m, this);
}
void ofxNetworkSyncServer::threadedFunction(){
	if(tcpServer != NULL){
		while(isThreadRunning() && finderResponder.IsConnected()){
			char recv[16];
			int num = finderResponder.Receive(recv, sizeof(recv));
			if(num > 0){
				string recvMessage = recv;
				if(recvMessage == UDP_MESSAGE_HELLO){
					char addr[16];
					if(finderResponder.GetRemoteAddr(addr)){
						ofxUDPManager responder;
						string sendMessage = UDP_MESSAGE_HELLO + " " + ofToString(tcpServer->getPort());
						responder.Create();
						responder.Connect(addr, finderSendPort);
						responder.Send(sendMessage.c_str(), sendMessage.length());
						responder.Close();
					}
				}
			}
			ofSleepMillis(10);
		}
	}
}

void ofxNetworkSyncServer::setTimeOffsetMillis(long long _timeOffset){
	timeOffset = _timeOffset;
}

long long ofxNetworkSyncServer::getSyncedElapsedTimeMillis(){
	return ofGetElapsedTimeMillis()+timeOffset;
}
long long ofxNetworkSyncServer::getTimeOffsetMillis(){
	return timeOffset;
}

vector<ofxNetworkSyncClientState *> & ofxNetworkSyncServer::getClients(){
	return clientStates;
}

void ofxNetworkSyncServer::setAutoCalibration(bool b){
	bAutoCalibration = b;
}
bool ofxNetworkSyncServer::getAutoCalibration(){
	return bAutoCalibration;
}
bool ofxNetworkSyncServer::isConnected(){
	if(tcpServer == NULL){
		return false;
	}
	return tcpServer->isConnected();
}
bool ofxNetworkSyncServer::hasClients(){
	return clientStates.size() > 0;
}


void ofxNetworkSyncServer::onClientDisconnected(int & clientId){
	for(vector<ofxNetworkSyncClientState *>::iterator it = clientStates.begin(); it != clientStates.end(); ){
		ofxNetworkSyncClientState * s = *it;
		if(s->getClientID() == clientId){
			delete s;
			it = clientStates.erase(it);
		}else{
			it++;
		}
	}
}



void ofxNetworkSyncServer::startRecalibration(){
	for (auto & state : clientStates) {
		state->startCalibration();
	}
	
}