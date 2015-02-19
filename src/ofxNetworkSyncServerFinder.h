//
//  ofxNetworkSyncServerFinder.h
//  ofxNetworkSyncServerFinderExample
//
//  Created by FURUDATE Ken on 2/19/15.
//
//

#ifndef ofxNetworkSyncServerFinderExample_ofxNetworkSyncServerFinder_h
#define ofxNetworkSyncServerFinderExample_ofxNetworkSyncServerFinder_h

#include "ofxNetwork.h"
#include "ofxNetworkSyncConstants.h"

class ofxNetworkSyncServerFinder : ofThread{
	ofxUDPManager udpSend, udpRecv;
	long long startTimeMillis, lastTryTimeMillis;
public:
	ofEvent<IpAndPort> serverFound;
	
	bool setup(int sendPort=FINDER_SEND_PORT_DEFAULT, int recvPort=FINDER_RESPOND_PORT_DEFAULT){
		if(udpSend.Create() &&
		   udpSend.SetEnableBroadcast(true) &&
		   udpSend.Connect("255.255.255.255", sendPort)){
		}else{
			ofLogError("ofxNetworkSyncServerFinder") << "failed to connect to port: " << sendPort;
			return false;
		}
		if(udpRecv.Create() &&
		   udpRecv.Bind(recvPort) &&
		   udpRecv.SetNonBlocking(true)
		   ){
		}else{
			ofLogError("ofxNetworkSyncServerFinder") << "failed to bind to port: " << recvPort;
			return false;
		}
		ofLogVerbose("ofxNetworkSyncServerFinder") << "server finder connect!" << endl;
		startThread(true);
		lastTryTimeMillis = startTimeMillis = 0;
		return true;
	}
	void close(){
		ofLogVerbose("ofxNetworkSyncServerFinder") << "closing server finder" << endl;
		stopThread();
		waitForThread();
		udpSend.Close();
		udpRecv.Close();
		ofLogVerbose("ofxNetworkSyncServerFinder") << "server finder closed" << endl;
	}
	bool isRunning(){
		return isThreadRunning();
	}
	
protected:
	void threadedFunction(){
		ofSleepMillis(1000);
		ofLogVerbose("ofxNetworkSyncServerFinder") << "start finding thread";
		startTimeMillis = ofGetElapsedTimeMillis();
		while (isThreadRunning()) {
			int now = ofGetElapsedTimeMillis();
			lastTryTimeMillis = now;
			udpSend.Send(UDP_MESSAGE_HELLO.c_str(), UDP_MESSAGE_HELLO.length());
			char recv[32];
			int num = udpRecv.Receive(recv, sizeof(recv));
			if(num > 0){
				string strRecv = recv;
				if(strRecv.find(UDP_MESSAGE_HELLO) == 0){
					int port = ofToInt(strRecv.substr(UDP_MESSAGE_HELLO.length()+1));
					char remoteAddr[16];
					udpRecv.GetRemoteAddr(remoteAddr);
					string strRemoteAddr = remoteAddr;
					IpAndPort info = {strRemoteAddr, port};
					ofNotifyEvent(serverFound, info, this);
				}
			}
			ofSleepMillis(SERVER_FIND_INTERVAL);
		}
	}
};

#endif
