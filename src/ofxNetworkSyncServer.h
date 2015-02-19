//
//  ofxNetworkSyncServer.h
//  toposcan_4screen
//
//  Created by FURUDATE Ken on 1/17/15.
//
//

#ifndef toposcan_4screen_ofxNetworkSyncServer_h
#define toposcan_4screen_ofxNetworkSyncServer_h

#include "ofMain.h"
#include "ofxNetwork.h"
#include "ofxNetworkSyncConstants.h"
#include "ofxNetworkSyncClientState.h"

typedef struct{
	int clientId;
	string message;
}ofxNetworkSyncServerMessage;

class ofxNetworkSyncServer : ofThread{
public:
	ofEvent<ofxNetworkSyncServerMessage> messageReceived;
	ofEvent<int> newClientConnected;
	
	ofxNetworkSyncServer();
	~ofxNetworkSyncServer();
	
	bool setup(int tcpPort, bool _bAutoCalibration=true,
			   int _finderRecvPort=FINDER_SEND_PORT_DEFAULT, int _finderSendPort=FINDER_RESPOND_PORT_DEFAULT);
	void close();
	void update();
	
	void drawStatus();
	
	vector<ofxNetworkSyncClientState *> & getClients(){
		return clientStates;
	}
	
	void setAutoCalibration(bool b){
		bAutoCalibration = b;
	}
	bool getAutoCalibration(){
		return bAutoCalibration;
	}
	bool isConnected(){
		if(tcpServer == NULL){
			return false;
		}
		return tcpServer->isConnected();
	}
	bool hasClients(){
		return clientStates.size() > 0;
	}
	
	void onClientMessageReceived(int clientId, string message);
protected:
	ofxTCPServer * tcpServer;
	ofxUDPManager finderResponder;
	int finderRecvPort;
	int finderSendPort;
	int tcpLastConnectionID;
	vector<ofxNetworkSyncClientState *> clientStates;
	
	bool bAutoCalibration;
	
	void threadedFunction();

};


#endif
