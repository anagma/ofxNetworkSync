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
	
	void drawStatus(int x=50, int y=50);
	
	vector<ofxNetworkSyncClientState *> & getClients();
	
	void setAutoCalibration(bool b);
	bool getAutoCalibration();
	bool isConnected();
	bool hasClients();
	
	void onClientMessageReceived(int clientId, string message);
	
	void setTimeOffsetMillis(long long _timeOffset);
	long long getSyncedElapsedTimeMillis();
	long long getTimeOffsetMillis();
	
	void startRecalibration();
protected:
	ofxTCPServer * tcpServer;
	ofxUDPManager finderResponder;
	bool bResponderConnected;

	int finderRecvPort;
	int finderSendPort;
	//int tcpLastConnectionID;
	
	int timeOffset;
	
	vector<ofxNetworkSyncClientState *> clientStates;
	
	bool bAutoCalibration;
	
	void threadedFunction();
	void onClientDisconnected(int & clientId);

};


#endif
