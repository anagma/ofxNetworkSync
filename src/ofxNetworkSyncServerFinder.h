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
	long long startTimeMillis;
	
	bool bServerFound;
	IpAndPort serverIpAndPort;
	
	
public:
	//ofEvent<IpAndPort> serverFound;
	
	
	~ofxNetworkSyncServerFinder(){
		close();
	}

	// ======================================================
	bool setup(int sendPort=FINDER_SEND_PORT_DEFAULT, int recvPort=FINDER_RESPOND_PORT_DEFAULT){
		close();

		bool bConnected = false;
		if(udpSend.Create() &&
		   udpSend.SetEnableBroadcast(true) &&
		   udpSend.Connect("255.255.255.255", sendPort)){
			bConnected = true;
		}else{
			ofLogError("ofxNetworkSyncServerFinder") << "FAILED TO CONNECT TO PORT: " << sendPort;
			bConnected = false;
		}
		if(udpRecv.Create() &&
		   udpRecv.Bind(recvPort) &&
		   udpRecv.SetNonBlocking(true)
		   ){
			bConnected = true;
		}else{
			ofLogError("ofxNetworkSyncServerFinder") << "FAILED BIND TO PORT: " << recvPort;
			bConnected = false;
		}
		
		if(bConnected){
			ofLogVerbose("ofxNetworkSyncServerFinder") << "CONNECTED" << endl;
			startTimeMillis = 0;
			bServerFound = false;
			if(! isThreadRunning()){
				ofLogVerbose("ofxNetworkSyncServerFinder") << "START THREAD!!" << endl;
				startThread(true);
			}
			return true;
		}else{
			close();
			return false;
		}
	}
	
	// ======================================================
	void close(){
		if(isThreadRunning()){
			ofLogVerbose("ofxNetworkSyncServerFinder") << "STOP THREAD!!" << endl;
			stopThread();
			waitForThread(true, 1000);
			ofLogVerbose("ofxNetworkSyncServerFinder") << "THREAD STOPPED!!" << endl;
		}
		if(udpSend.IsConnected()){
			udpSend.Close();
			ofLogVerbose("ofxNetworkSyncServerFinder") << "SENDER CLOSED" << endl;
		}
		if(udpRecv.IsConnected()){
			udpRecv.Close();
			ofLogVerbose("ofxNetworkSyncServerFinder") << "RECEIVER CLOSED" << endl;
		}
	}
	
	// ======================================================
	bool doesServerFound(){
		return bServerFound;
	}
	IpAndPort getServerInfo(){
		return serverIpAndPort;
	}
	
	// ======================================================
	bool isConnected(){
		return udpSend.IsConnected() && udpRecv.IsConnected();
	}
	
	bool isRunning(){
		return isThreadRunning();
	}

	
protected:
	void threadedFunction(){
		ofLogVerbose("ofxNetworkSyncServerFinder") << "THREAD STARTED";
		ofSleepMillis(1000);
		startTimeMillis = ofGetElapsedTimeMillis();
		while (isThreadRunning()) {
			if(! isConnected()){
				ofLogError("ofxNetworkSyncServerFinder") << "CONNECTION LOST";
				break;
			}
			
			ofLogVerbose("ofxNetworkSyncServerFinder") << "SENDING HELLO";
			udpSend.Send(UDP_MESSAGE_HELLO.c_str(), UDP_MESSAGE_HELLO.length());
			char recv[32];
			int num = udpRecv.Receive(recv, sizeof(recv));
			if(num > 0){
				ofLogVerbose("ofxNetworkSyncServerFinder") << "MESSAGE RECEIVED: " << recv;
				istringstream istr(recv);
				string header;
				istr >> header;
				
				if(header == UDP_MESSAGE_HELLO){
					ofLogNotice("ofxNetworkSyncServerFinder") << "SERVER FOUND";
					string strPort;
					istr >> strPort;
					int port = ofToInt(strPort);
					char remoteAddr[16];
					udpRecv.GetRemoteAddr(remoteAddr);
					serverIpAndPort.ip		= remoteAddr;
					serverIpAndPort.port	= port;
					bServerFound = true;
//					break;
				}
			}
			ofSleepMillis(SERVER_FIND_INTERVAL);
		}
//		if(bServerFound){
//			ofLogNotice("ofxNetworkSyncServerFinder") << "NOTIFY";
//			ofNotifyEvent(serverFound, serverIpAndPort, this);
//		}
		ofLogVerbose("ofxNetworkSyncServerFinder") << "THREAD END";
	}
};

#endif
