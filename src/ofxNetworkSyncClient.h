//
//  ofxNetworkSyncClient.h
//  toposcan_4screen
//
//  Created by FURUDATE Ken on 1/17/15.
//
//

#ifndef toposcan_4screen_ofxNetworkSyncClient_h
#define toposcan_4screen_ofxNetworkSyncClient_h

#include "ofMain.h"
#include "ofxNetwork.h"
#include "ofxNetworkSyncConstants.h"
#include "ofxNetworkSyncUdp.h"

class ofxNetworkSyncClient : ofThread{
	string serverIp;
	int serverPort;
	ofxTCPClient client;
	ofxNetworkSyncUdpResponder calibrator;
	
	int clientId;
	
	// result
	int latency;
	int timeDifference;
		
	ofxNetworkSyncCalibrationStep step;
	
public:
	ofEvent<string> messageReceived;

	ofxNetworkSyncClient() : ofThread(){
		step = WAIT;
	}
	~ofxNetworkSyncClient(){
		if(client.isConnected()){
			client.close();
		}
	}
	
	bool setup(string serverIp_, int serverPort_){
		serverIp = serverIp_;
		serverPort = serverPort_;

		return connect();
	}
	bool connect(){
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
	bool close(){
		ofLogVerbose("ofxNetworkSyncClient") << "Close connection";
		return client.close();
	}
	
	void drawStatus(){
		ostringstream ostr("");
		
		if(isConnected()){
			ostr << "Client is connected to " << serverIp << ":" << serverPort << endl;
			switch (step) {
				case WAIT:
					ostr << "NOT CALIBRATED" << endl;
					break;
				case CALIBRATING:
					ostr << "CALIBRATING NOW" << endl;
					for(auto & c : calibrator.getTimeDifferences()){
						ostr << "differences:" << c << endl;
					}
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
		
		
		ofSetColor(255);
		ofDrawBitmapString(ostr.str(), 50, 50);
	}
	
	long long getSyncedElapsedTimeMillis(){
		return ofGetElapsedTimeMillis() - timeDifference;
	}
	float getLatency(){
		return latency;
	}
	unsigned long long getSyncedBaseTimeMillis(){
		return timeDifference;
	}

	bool isConnected(){
		return client.isConnected();
	}
	bool isCalibrated(){
		return isConnected() && step == CALIBRATED;
	}
	bool isCalibrating(){
		return isConnected() && step == CALIBRATING;
	}
protected:
	void threadedFunction(){
		while (isThreadRunning() && isConnected()) {
			string recv = client.receive();
			if(recv.length() > 0){
				ofNotifyEvent(messageReceived, recv, this);
			}
		}
	}
	void onMessageReceived(string & message){
		if(message.find(MESSAGE_HEADER_CLIENT_ID) == 0){
			ofLogVerbose("ofxNetworkSyncClient") << "set client ID: " << clientId;
			clientId = ofToInt(message.substr(MESSAGE_HEADER_CLIENT_ID.length()+1));
		}else if(message == MESSAGE_START_REQUEST){
			ofLogVerbose("ofxNetworkSyncClient") << "start signal received, send responce and prepare";
			
			if(calibrator.setup(serverIp, SERVER_SEND_PORT_OFFSET+clientId, SERVER_RECV_PORT_OFFSET+clientId)){
				step = CALIBRATING;
				client.send(MESSAGE_START_RESPONCE);
			}else{
				step = WAIT;
				ofLogError("ofxNetworkSyncClient") << "Failed to start calibrator";
				client.send(MESSAGE_START_FAILED);
			}
		}else if(message.find(MESSAGE_HEADER_RESULT) == 0){
			ofLogVerbose("ofxNetworkSyncClient") << "stop signal received. it seems that calibration is complete.";
			calibrator.close();
			
			latency = ofToInt(message.substr(MESSAGE_HEADER_RESULT.length()+1));
			ofLogVerbose("ofxNetworkSyncClient") << "latency received: " << latency;
			
			timeDifference = round(calibrator.getTimeDifference() - latency);
			ofLogVerbose("ofxNetworkSyncClient") << "time difference calicurated: " << timeDifference;
			ofLogVerbose("ofxNetworkSyncClient") << "done calibration";
			
			step = CALIBRATED;
		}
	}
};

#endif
