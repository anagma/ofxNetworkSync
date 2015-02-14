//
//  ofxNetworkSyncClientState.h
//  ofxNetworkSyncServerExample
//
//  Created by FURUDATE Ken on 1/18/15.
//
//

#ifndef ofxNetworkSyncServerExample_ofxNetworkSyncClientState_h
#define ofxNetworkSyncServerExample_ofxNetworkSyncClientState_h

#include "ofMain.h"
#include "ofxNetwork.h"
#include "ofxNetworkSyncConstants.h"
#include "ofxNetworkSyncUdp.h"

class ofxNetworkSyncServer;

class ofxNetworkSyncClientState : ofThread{
	
	
	int clientId;
	string ip;
	int port;
	
	ofxTCPServer * tcpServer;
	ofxNetworkSyncServer * server;
	ofxNetworkSyncUdpSender calibrator;
	
	ofxNetworkSyncCalibrationStep step;
//	
//	bool bCalibrated;
	
public:
	ofEvent<string> messageReceived;

	ofxNetworkSyncClientState(ofxNetworkSyncServer * _server, ofxTCPServer * tcpServer_, const int clientId_);
	~ofxNetworkSyncClientState();

	void startCalibration();
	void stopCalibration();
	bool close();
	void send(string message);
	
	bool isConnected(){
		return tcpServer->isClientConnected(clientId);
	}
	
	bool isCalibrated(){
		return isConnected() && step == CALIBRATED;
	}
	bool isCalibrating(){
		return isConnected() && calibrator.isRunning();
	}
	string getIpAddr(){
		return ip;
	}
	int getPort(){
		return port;
	}
	int getClientID(){
		return clientId;
	}
	
protected:
	void threadedFunction();
	void onMessageReceived(string & message);};

#endif
