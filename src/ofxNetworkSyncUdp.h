//
//  ofxNetworkSyncUdpServer.h
//  toposcan_4screen
//
//  Created by FURUDATE Ken on 1/21/15.
//
//

#ifndef toposcan_4screen_ofxNetworkSyncUdpServer_h
#define toposcan_4screen_ofxNetworkSyncUdpServer_h

#include "ofMain.h"
#include "ofxNetwork.h"
#include "ofxNetworkSyncConstants.h"


class ofxNetworkSyncUdpBase : public ofThread{
	
	int startTime;
	
public:
	
	bool setup(string host, int recvPort, int sendPort);
	void close();
	bool isRunning(){
		return isThreadRunning();
	}
	int getRunningTime(){
		return ofGetElapsedTimeMillis() - startTime;
	}
	bool isConnected(){
		return bConnected;
	}
protected:
	ofxUDPManager udpRecv, udpSend;
	bool bConnected;
	
	void threadedFunction();
};

// ====================================================================================

class ofxNetworkSyncUdpSender : public ofxNetworkSyncUdpBase{
	
	list<int> latencies;
	int numMeasurement;
	
public:
	ofEvent<void> finishMeasuremnt;
	
	bool setup(string host, int recvPort, int sendPort, int _numMeasurement=NUM_MEASUREMENT_DEFAULT);
	list<int> & getLatencies(){
		return latencies;
	}
	float getLatency();
	int getLatencyResponceCount(){
		return latencies.size();
	}
protected:
	void threadedFunction();
};

// ====================================================================================

class ofxNetworkSyncUdpResponder : public ofxNetworkSyncUdpBase{

	list<int> timeDifferences;
	
public:
	
	bool setup(string host, int recvPort, int sendPort);
	list<int> & getTimeDifferences(){
		return timeDifferences;
	}
	float getTimeDifference();	int getTimeDifferenceLogCount(){
		return timeDifferences.size();
	}
protected:
	void threadedFunction();};
#endif
