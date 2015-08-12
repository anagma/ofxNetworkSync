//
//  ofxNetworkSyncClientState.cpp
//  networkSyncVideoPlayer
//
//  Created by FURUDATE Ken on 2/15/15.
//
//

#include "ofxNetworkSyncClientState.h"
#include "ofxNetworkSyncServer.h"

ofxNetworkSyncClientState::ofxNetworkSyncClientState(ofxNetworkSyncServer * _server, ofxTCPServer * tcpServer_, const int clientId_) : ofThread(), server(_server), tcpServer(tcpServer_), clientId(clientId_){
	ip = tcpServer->getClientIP(clientId);
	port = tcpServer->getClientPort(clientId);
	
	ofAddListener(messageReceived, this, &ofxNetworkSyncClientState::onMessageReceived);
	ofLogVerbose("ofxNetworkSyncClientState") << "Client state created. send client id to client and start thread";
	send(MESSAGE_HEADER_CLIENT_ID+MESSAGE_HEADER_SEPARATOR+ofToString(clientId));
	step = WAIT;
	startThread(true);
}
ofxNetworkSyncClientState::~ofxNetworkSyncClientState(){
	close();
}

void ofxNetworkSyncClientState::startCalibration(){
	ofLogVerbose("ofxNetworkSyncClientState#"+ofToString(clientId)) << "sending start signal";
	send(MESSAGE_START_REQUEST);
	
	
}
void ofxNetworkSyncClientState::stopCalibration(){
	calibrator.close();
	
	float result = calibrator.getLatency();
	ofLogVerbose("ofxNetworkSyncClientState#"+ofToString(clientId)) << "latency is: " << round(result);
	ofLogVerbose("ofxNetworkSyncClientState#"+ofToString(clientId)) << "sending time offset: " << server->getTimeOffsetMillis();
	
	send(MESSAGE_HEADER_RESULT+MESSAGE_HEADER_SEPARATOR+ofToString(round(result)));
	send(MESSAGE_HEADER_TIME_OFFSET+MESSAGE_HEADER_SEPARATOR+ofToString(server->getTimeOffsetMillis()));
	step = CALIBRATED;
}


bool ofxNetworkSyncClientState::close(){
	if(isThreadRunning()){
		stopThread();
		waitForThread();
	}
	calibrator.close();
	server = NULL;
	tcpServer = NULL;
}
void ofxNetworkSyncClientState::send(string message){
	if(isConnected()){
		tcpServer->send(clientId, message);
	}else{
		ofLogWarning("ofxNetworkSyncClientState#"+ofToString(clientId)) << "client is not connected";
	}
}

void ofxNetworkSyncClientState::threadedFunction(){
	while(isThreadRunning()){
		if(step == CALIBRATING && calibrator.isFinishMeasuremant()){
			stopCalibration();
		}
		
		if(! isConnected()){
			ofNotifyEvent(clientDisconnected, clientId, this);
			break;
		}
		string recv = tcpServer->receive(clientId);
		if(recv.length() > 0){
			ofNotifyEvent(messageReceived, recv, this);
			server->onClientMessageReceived(clientId, recv);
		}
		ofSleepMillis(10);
	}
	
}
void ofxNetworkSyncClientState::onMessageReceived(string & message){
	if(message == MESSAGE_START_RESPONCE){
		ofLogVerbose("ofxNetworkSyncClientState#"+ofToString(clientId)) << "responce received";
		step = CALIBRATING;
		calibrator.setup(ip, SERVER_RECV_PORT_OFFSET+clientId, SERVER_SEND_PORT_OFFSET+clientId);
//		ofAddListener(calibrator.finishMeasuremnt, this, &ofxNetworkSyncClientState::stopCalibration);
	}else if(message == MESSAGE_START_FAILED){
		ofLogError("ofxNetworkSyncClientState#"+ofToString(clientId)) << "start calibration failed";
		step = WAIT;
	}
}



bool ofxNetworkSyncClientState::isConnected(){
	return tcpServer->isClientConnected(clientId);
}

bool ofxNetworkSyncClientState::isCalibrated(){
	return isConnected() && step == CALIBRATED;
}
bool ofxNetworkSyncClientState::isCalibrating(){
	return isConnected() && calibrator.isRunning();
}
string ofxNetworkSyncClientState::getIpAddr(){
	return ip;
}
int ofxNetworkSyncClientState::getPort(){
	return port;
}
int ofxNetworkSyncClientState::getClientID(){
	return clientId;
}