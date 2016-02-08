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
	ofEvent<void> connectionLost;
	ofEvent<void> calibrated;

	ofxNetworkSyncClient();
	~ofxNetworkSyncClient();
	
	bool setup(string serverIp_, int serverPort_);
	bool connect();
	bool close();
	void drawStatus(int x=50, int y=50);
	
	// network utils
	int getRemotePort();
	string getRemoteHost();
	
	
	long long getSyncedElapsedTimeMillis();
	float getLatency();
	long long getSyncedBaseTimeMillis();

	bool isConnected();
	bool isCalibrated();
	bool isCalibrating();

	void send(string message);
	
	int getClientId();
	void startRecalibration();
protected:
	void startCalibration();
	void threadedFunction();
	void onMessageReceived(string & message);
};

#endif
