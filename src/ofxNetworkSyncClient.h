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

	ofxNetworkSyncClient();
	~ofxNetworkSyncClient();
	
	bool setup(string serverIp_, int serverPort_);
	bool connect();
	bool close();
	void drawStatus();
	
	long long getSyncedElapsedTimeMillis(){
		if(! isCalibrated()){
			ofLogWarning("ofxNetworkSyncClient") << "is not calibrated yet..";
		}
		return ofGetElapsedTimeMillis() - timeDifference;
	}
	float getLatency(){
		if(! isCalibrated()){
			ofLogWarning("ofxNetworkSyncClient") << "is not calibrated yet..";
		}
		return latency;
	}
	long long getSyncedBaseTimeMillis(){
		if(! isCalibrated()){
			ofLogWarning("ofxNetworkSyncClient") << "is not calibrated yet..";
		}
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
	void threadedFunction();
	void onMessageReceived(string & message);
};

#endif
