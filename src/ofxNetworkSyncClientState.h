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
#include "ofxNetworkSyncConstants.h"
#include "ofxNetworkSyncUdp.h"

class ofxNetworkSyncClientState : ofThread{
	
	
	int clientId;
	string ip;
	int port;
	
	ofxTCPServer * tcpServer;
	ofxNetworkSyncUdpSender calibrator;
	
	ofxNetworkSyncCalibrationStep step;
//	
//	bool bCalibrated;
	
public:
	ofEvent<string> messageReceived;

	ofxNetworkSyncClientState(ofxTCPServer * tcpServer_, const int clientId_) : ofThread(), tcpServer(tcpServer_), clientId(clientId_){
		ip = tcpServer->getClientIP(clientId);
		port = tcpServer->getClientPort(clientId);
		
		ofAddListener(messageReceived, this, &ofxNetworkSyncClientState::onMessageReceived);
		ofLogVerbose("ofxNetworkSyncClientState") << "Client state created. send client id to client and start thread";
		send(MESSAGE_HEADER_CLIENT_ID+MESSAGE_HEADER_SEPARATOR+ofToString(clientId));
		step = WAIT;
		startThread(true);
	}
	~ofxNetworkSyncClientState(){
		
	}

	void startCalibration(){
		ofLogVerbose("ofxNetworkSyncClientState#"+ofToString(clientId)) << "sending start signal";
		send(MESSAGE_START_REQUEST);
		
		
	}
	void stopCalibration(){
		calibrator.close();
		
		float result = calibrator.getLatency();
		ofLogVerbose("ofxNetworkSyncClientState#"+ofToString(clientId)) << "latency is: " << round(result);
		
		send(MESSAGE_HEADER_RESULT+MESSAGE_HEADER_SEPARATOR+ofToString(round(result)));
		step = CALIBRATED;
	}
	
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
	
	bool close(){
		if(isThreadRunning()){
			stopThread();
		}
	}
	void send(string message){
		if(isConnected()){
			tcpServer->send(clientId, message);
		}else{
			ofLogWarning("ofxNetworkSyncClientState#"+ofToString(clientId)) << "client is not connected";
		}
	}
	
protected:
	void threadedFunction(){
		while(isThreadRunning() && isConnected()){
			string recv = tcpServer->receive(clientId);
			if(recv.length() > 0){
				ofNotifyEvent(messageReceived, recv, this);
			}
		}
		
	}
	void onMessageReceived(string & message){
		if(message == MESSAGE_START_RESPONCE){
			ofLogVerbose("ofxNetworkSyncClientState#"+ofToString(clientId)) << "responce received";
			step = CALIBRATING;
			calibrator.setup(ip, SERVER_RECV_PORT_OFFSET+clientId, SERVER_SEND_PORT_OFFSET+clientId);
			ofAddListener(calibrator.finishMeasuremnt, this, &ofxNetworkSyncClientState::stopCalibration);
		}else if(message == MESSAGE_START_FAILED){
			ofLogError("ofxNetworkSyncClientState#"+ofToString(clientId)) << "start calibration failed";
			step = WAIT;
		}
	}
};

#endif
