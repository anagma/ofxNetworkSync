//
//  ofxNetworkSyncUdp.cpp
//  networkSyncVideoPlayer
//
//  Created by FURUDATE Ken on 2/15/15.
//
//

#include "ofxNetworkSyncUdp.h"

bool ofxNetworkSyncUdpBase::setup(string host, int recvPort, int sendPort){
	
	// ===================== CREATE RECEIVER
	if(! udpRecv.Create()){
		ofLogError("ofxNetworkSyncUdpBase") << "Failed to create udp receiver";
		return false;
	}
	if (udpRecv.Bind(recvPort)) {
		udpRecv.SetNonBlocking(true);
		ofLogVerbose("ofxNetworkSyncUdpBase") << "Binded udp receiver on port: " << recvPort;
	}else{
		ofLogError("ofxNetworkSyncUdpBase") << "Failed to bind on port: " << recvPort;
		return false;
	}
	
	// ===================== CREATE SENDER
	if(! udpSend.Create()){
		udpSend.SetNonBlocking(true);
		ofLogError("ofxNetworkSyncUdpBase") << "Failed to create udp sender";
		return false;
	}
	if(udpSend.Connect(host.c_str(), sendPort)){
		ofLogVerbose("ofxNetworkSyncUdpBase") << "Connected udp sender to: " << host << ":" << sendPort;
	}else{
		ofLogError("ofxNetworkSyncUdpBase") << "Failed to connect to server: " << host << ":" << sendPort;
		close(); // close receiver too.
		return false;
	}
	
	bConnected = true;
	
	// ===================== START CALIBRATION THREAD
	startThread(true);
	startTime = ofGetElapsedTimeMillis();
	return true;
}

void ofxNetworkSyncUdpBase::close(){
	ofLogVerbose("ofxNetworkSyncUdpBase") << "Close down udp connection";
	closeConnection();
	if(isThreadRunning()){
		stopThread();
		waitForThread(true, 1000);
	}
}
void ofxNetworkSyncUdpBase::closeConnection(){
	udpRecv.Close();
	udpSend.Close();
}
void ofxNetworkSyncUdpBase::threadedFunction(){
	
}

// ====================================================================================

bool ofxNetworkSyncUdpSender::setup(string host, int recvPort, int sendPort, int _numMeasurement){
	numMeasurement = _numMeasurement;
//	ofAddListener(finishMeasuremnt, this, &ofxNetworkSyncUdpSender::onFinishMeasuremnt);
	latencies.clear();
	bFinishMeasurement = false;
	return ofxNetworkSyncUdpBase::setup(host, recvPort, sendPort);
}
float ofxNetworkSyncUdpSender::getLatency(){
	// calculate...
	if(getLatencyResponceCount() < 2){
		ofLogError("ofxNetworkSyncUdpSender") << "Data is not enough... " << getLatencyResponceCount();
		return 0;
	}
	
	//		// trim min and max
	//		latencies.sort();
	//		latencies.pop_back();
	//		latencies.pop_front();
	
	float result = 0;
	for(auto & l : latencies){
		result += l;
	}
	result /= latencies.size();
	return result;
}
void ofxNetworkSyncUdpSender::threadedFunction(){
	ofSleepMillis(1000);
	
	const int messLength = 32;
	
	char messRecv[messLength];
	bool bSending = false;
	int lastSendTime = ofGetElapsedTimeMillis();
	bool bFinish = false;
	
	while(isThreadRunning()){
		
		if(! isConnected()){
			ofLogError("ofxNetworkSyncUdpSender") << "Lost connection unexpectedly";
			break;
		}
		if(! bSending){
			lastSendTime = ofGetElapsedTimeMillis();
			ofLogVerbose("ofxNetworkSyncUdpSender") << "Sending Timestamp: " << lastSendTime;
			string messSend = UDP_MESSAGE_SEND+ofToString(lastSendTime);
			udpSend.Send(messSend.c_str(), messSend.length());
			bSending = true;
			//ofSleepMillis(10);
		}
		if(lastSendTime+1000 < ofGetElapsedTimeMillis()){
			ofLogNotice("ofxNetworkSyncUdpSender") << "Timeout!!";
			bSending = false;
		}
		if(bSending){
			udpRecv.Receive(messRecv, messLength);
			string strMessRecv = messRecv;
			if(strMessRecv.length() > 0){
				if(strMessRecv.find(UDP_MESSAGE_REPLY) == 0){
					int recvTime = ofToInt(strMessRecv.substr(UDP_MESSAGE_REPLY.length()));
					ofLogVerbose("ofxNetworkSyncUdpSender") << "Received Reply: " << strMessRecv;
					if(recvTime > 0 && recvTime == lastSendTime){
						latencies.push_back((ofGetElapsedTimeMillis() - lastSendTime)*0.5);
						ofLogVerbose("ofxNetworkSyncUdpSender") << "Latency: " << latencies.back();
						bSending = false;
						if(latencies.size() >= numMeasurement){
							bFinishMeasurement = true;
							break;
						}
					}
				}
			}
		}
	}
	closeConnection();
}

// ====================================================================================


bool ofxNetworkSyncUdpResponder::setup(string host, int recvPort, int sendPort){
	timeDifferences.clear();
	return ofxNetworkSyncUdpBase::setup(host, recvPort, sendPort);
}
float ofxNetworkSyncUdpResponder::getTimeDifference(){
	if(getTimeDifferenceLogCount() < 2){
		ofLogError("ofxNetworkSyncUdpResponder") << "Data is not enough... " << getTimeDifferenceLogCount();
		return 0;
	}
	
	//		// trim min and max
	//		timeDifferences.sort();
	//		timeDifferences.pop_front();
	//		timeDifferences.pop_back();
	
	float diffAvg = 0;
	for(auto & t : timeDifferences){
		diffAvg += t;
	}
	diffAvg /= timeDifferences.size();
	return diffAvg;
}
void ofxNetworkSyncUdpResponder::threadedFunction(){
	
	char messRecv[32];
	
	while(isThreadRunning()){
		if(! isConnected()){
			ofLogError("ofxNetworkSyncUdpResponder") << "Lost connection unexpectedly";
			closeConnection();
			break;
		}
		
		udpRecv.Receive(messRecv, 32);
		string strMessRecv = messRecv;
		if(strMessRecv.length() > 0){
			if(strMessRecv.find(UDP_MESSAGE_SEND) == 0){
				int recvTime = ofToInt(strMessRecv.substr(UDP_MESSAGE_SEND.length()));
				ofLogVerbose("ofxNetworkSyncUdpResponder") << "Received Timestamp: " << strMessRecv << ", return it.";
				if(recvTime > 0){
					int now = ofGetElapsedTimeMillis();
					timeDifferences.push_back(now-recvTime);
					ofLogVerbose("ofxNetworkSyncUdpResponder") << "Time difference: " << timeDifferences.back();
					string messSend = UDP_MESSAGE_REPLY+ofToString(recvTime);
					udpSend.Send(messSend.c_str(), messSend.length());
				}
			}
		}
	}
}
