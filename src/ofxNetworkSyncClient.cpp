//
//  ofxNetworkSyncClient.cpp
//  networkSyncVideoPlayer
//
//  Created by FURUDATE Ken on 2/15/15.
//
//

#include "ofxNetworkSyncClient.h"

	
ofxNetworkSyncClient::ofxNetworkSyncClient() : ofThread(){
	step = WAIT;
}
ofxNetworkSyncClient::~ofxNetworkSyncClient(){
	close();
}

bool ofxNetworkSyncClient::setup(string serverIp_, int serverPort_){
	serverIp = serverIp_;
	serverPort = serverPort_;
	
	return connect();
}
bool ofxNetworkSyncClient::connect(){
	if(client.setup(serverIp, serverPort)){
		ofLogVerbose("ofxNetworkSyncClient") << "Client connect to " << serverIp << ":" << serverPort << ". stand by for calibration.";
		ofAddListener(messageReceived, this, &ofxNetworkSyncClient::onMessageReceived);
		startThread(true);
		return true;
	}else{
		ofLogError("ofxNetworkSyncClient") << "Failed to connect to server";
		return false;
	}
}
bool ofxNetworkSyncClient::close(){
	ofLogVerbose("ofxNetworkSyncClient") << "Close connection";
	if(isThreadRunning()){
		stopThread();
		waitForThread();
	}
	if(client.isConnected()){
		return client.close();
	}
	return true;
}

void ofxNetworkSyncClient::drawStatus(int x, int y){
	ostringstream ostr("");
	
	if(isConnected()){
		ostr << "Client is connected to " << serverIp << ":" << serverPort << endl;
		switch (step) {
			case WAIT:
				ostr << "NOT CALIBRATED" << endl;
				ostr << endl;
				ostr << endl;
				break;
			case CALIBRATING:
				ostr << "CALIBRATING NOW" << endl;
				for(auto & c : calibrator.getTimeDifferences()){
					ostr << "differences:" << c << endl;
				}
				ostr << endl;
				ostr << endl;
				break;
			case CALIBRATED:
				ostr << "CALIBRATED" << endl;
				ostr << "latency: " << latency << endl;
				ostr << "difference between me and server: " << timeDifference << endl;
				break;
				
			default:
				break;
		}
	}else{
		ostr << "Client is not connected." << endl;
	}
	ostr << "now: " << getSyncedElapsedTimeMillis() << endl;
	
	
	ofSetColor(255);
	ofDrawBitmapString(ostr.str(), x, y);
}

void ofxNetworkSyncClient::threadedFunction(){
	while (isThreadRunning()) {
		if(isConnected()){
			string recv = client.receive();
			if(recv.length() > 0){
				ofNotifyEvent(messageReceived, recv, this);
			}
		}else{
			ofNotifyEvent(connectionLost, this);
			break;
		}
		ofSleepMillis(10);
	}
}
void ofxNetworkSyncClient::onMessageReceived(string & message){
	if(message.find(MESSAGE_HEADER_CLIENT_ID) == 0){
		ofLogVerbose("ofxNetworkSyncClient") << "set client ID: " << clientId;
		clientId = ofToInt(message.substr(MESSAGE_HEADER_CLIENT_ID.length()+1));
	}else if(message == MESSAGE_START_REQUEST){
		ofLogVerbose("ofxNetworkSyncClient") << "start signal received, send responce and prepare";
		startCalibration();
	}else if(message.find(MESSAGE_HEADER_RESULT) == 0){
		ofLogVerbose("ofxNetworkSyncClient") << "stop signal received. it seems that calibration is complete.";
		calibrator.close();
		
		latency = ofToInt(message.substr(MESSAGE_HEADER_RESULT.length()+1));
		ofLogVerbose("ofxNetworkSyncClient") << "latency received: " << latency;
		
		timeDifference = round(calibrator.getTimeDifference() - latency);
		ofLogVerbose("ofxNetworkSyncClient") << "time difference calicurated: " << timeDifference;
		ofLogVerbose("ofxNetworkSyncClient") << "done calibration";

		ofNotifyEvent(calibrated, this);
		
		step = CALIBRATED;
		
	}else if(message.find(MESSAGE_HEADER_TIME_OFFSET) == 0){
		int timeOffset = ofToInt(message.substr(MESSAGE_HEADER_TIME_OFFSET.length()+1));
		ofLogVerbose("ofxNetworkSyncClient") << "time offset received:" << timeOffset;
		timeDifference -= timeOffset;
		ofLogVerbose("ofxNetworkSyncClient") << "calculated time difference:" << timeDifference;
	}
}



int ofxNetworkSyncClient::getRemotePort(){
	return client.getPort();
}
string ofxNetworkSyncClient::getRemoteHost(){
	return client.getIP();
}


long long ofxNetworkSyncClient::getSyncedElapsedTimeMillis(){
	if(! isCalibrated()){
		ofLogWarning("ofxNetworkSyncClient") << "is not calibrated yet..";
	}
	return ofGetElapsedTimeMillis() - timeDifference;
}
float ofxNetworkSyncClient::getLatency(){
	if(! isCalibrated()){
		ofLogWarning("ofxNetworkSyncClient") << "is not calibrated yet..";
	}
	return latency;
}
long long ofxNetworkSyncClient::getSyncedBaseTimeMillis(){
	if(! isCalibrated()){
		ofLogWarning("ofxNetworkSyncClient") << "is not calibrated yet..";
	}
	return timeDifference;
}

bool ofxNetworkSyncClient::isConnected(){
	return client.isConnected();
}
bool ofxNetworkSyncClient::isCalibrated(){
	return step == CALIBRATED;
}
bool ofxNetworkSyncClient::isCalibrating(){
	return isConnected() && step == CALIBRATING;
}
int ofxNetworkSyncClient::getClientId(){
	return clientId;
}
void ofxNetworkSyncClient::send(string message){
	client.send(message);
}
void ofxNetworkSyncClient::startRecalibration(){
	startCalibration();
}
void ofxNetworkSyncClient::startCalibration(){
	if(calibrator.setup(serverIp, SERVER_SEND_PORT_OFFSET+clientId, SERVER_RECV_PORT_OFFSET+clientId)){
		step = CALIBRATING;
		client.send(MESSAGE_START_RESPONCE);
	}else{
		step = WAIT;
		ofLogError("ofxNetworkSyncClient") << "Failed to start calibrator";
		client.send(MESSAGE_START_FAILED);
	}
}