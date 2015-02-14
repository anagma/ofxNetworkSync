//
//  ofxNetworkSyncServer.cpp
//  networkSyncVideoPlayer
//
//  Created by FURUDATE Ken on 2/15/15.
//
//

#include "ofxNetworkSyncServer.h"

ofxNetworkSyncServer::ofxNetworkSyncServer(){
	
}
ofxNetworkSyncServer::~ofxNetworkSyncServer(){
	close();
}

bool ofxNetworkSyncServer::setup(int tcpPort, bool _bAutoCalibration){
	
	bAutoCalibration = _bAutoCalibration;
	clientStates.clear();
	tcpLastConnectionID = -1;
	
	if(tcpServer.setup(tcpPort, false)){
		ofLogVerbose("ofxNetworkSyncServer") << "Server is waiting on port: " << tcpPort;
		return true;
	}else{
		ofLogError("ofxNetworkSyncServer") << "Failed to start server on port: " << tcpPort;
		return false;
	}
}
void ofxNetworkSyncServer::close(){
	ofLogVerbose("ofxNetworkSyncServer") << "close server... bye";
	tcpServer.close();
	for (auto & state : clientStates) {
		state->close();
		delete state;
	}
	clientStates.clear();
	tcpLastConnectionID = -1;
}
void ofxNetworkSyncServer::update(){
	if(! tcpServer.isConnected() || tcpServer.getLastID() <= 0){
		// no connection
		return;
	}
	
	if(tcpLastConnectionID != tcpServer.getLastID()){
		int clientId = tcpServer.getLastID()-1;
		ofLogVerbose("ofxNetworkSyncServer") << "New Connection established: #"<< clientId << " " << tcpServer.getClientIP(clientId) << ":" << tcpServer.getClientPort(clientId);
		// new connection
		clientStates.push_back(new ofxNetworkSyncClientState(this, &tcpServer, clientId));
		ofNotifyEvent(newClientConnected, clientId, this);
		
		tcpLastConnectionID = tcpServer.getLastID();
		
		// New Client will be calibrated automatically.
		if(bAutoCalibration){
			ofLogVerbose("ofxNetworkSyncServer") << "Client#" << clientId << " : Start Calibration";
			clientStates.back()->startCalibration();
		}
		//			ofNotifyEvent(newClientComming, clientStates.back(), this);
	}
}

void ofxNetworkSyncServer::drawStatus(){
	ostringstream ostr("");
	
	if(tcpServer.isConnected()){
		ostr << "Server is waiting on port:" << tcpServer.getPort() << endl;
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
				ostr << "CALIBRATED";
			}else{
				ostr << "NOT CALIBRATED";
			}
			ostr << endl;
		}
	}else{
		ostr << "Here is no clients." << endl;
	}
	
	ofSetColor(255);
	ofDrawBitmapString(ostr.str(), 50, 50);
}

void ofxNetworkSyncServer::onClientMessageReceived(int clientId, string message){
	ofxNetworkSyncServerMessage m = {clientId, message};
	ofNotifyEvent(messageReceived, m, this);
}