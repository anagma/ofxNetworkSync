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
		ofLogError("ofxNetworkSyncLatencyCalibrator") << "Failed to create udp receiver";
		return false;
	}
	if (udpRecv.Bind(recvPort)) {
		ofLogVerbose("ofxNetworkSyncLatencyCalibrator") << "Binded udp receiver on port: " << recvPort;
	}else{
		ofLogError("ofxNetworkSyncLatencyCalibrator") << "Failed to bind on port: " << recvPort;
		return false;
	}
	
	// ===================== CREATE SENDER
	if(! udpSend.Create()){
		ofLogError("ofxNetworkSyncLatencyCalibrator") << "Failed to create udp sender";
		return false;
	}
	if(udpSend.Connect(host.c_str(), sendPort)){
		ofLogVerbose("ofxNetworkSyncLatencyCalibrator") << "Connected udp sender to: " << host << ":" << sendPort;
	}else{
		ofLogError("ofxNetworkSyncLatencyCalibrator") << "Failed to connect to server: " << host << ":" << sendPort;
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
	ofLogVerbose("ofxNetworkSyncLatencyCalibrator") << "Close down udp connection";
	udpRecv.Close();
	udpSend.Close();
	if(isThreadRunning()){
		stopThread();
	}
}

void ofxNetworkSyncUdpBase::threadedFunction(){
	
}

// ====================================================================================

bool ofxNetworkSyncUdpSender::setup(string host, int recvPort, int sendPort, int _numMeasurement){
	numMeasurement = _numMeasurement;
	latencies.clear();
	return ofxNetworkSyncUdpBase::setup(host, recvPort, sendPort);
}
float ofxNetworkSyncUdpSender::getLatency(){
	// calculate...
	if(getLatencyResponceCount() < 2){
		ofLogError("ofxNetworkSyncLatencyCalibrator") << "Data is not enough... " << getLatencyResponceCount();
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
	
	while(isThreadRunning()){
		
		if(! isConnected()){
			ofLogError("ofxNetworkSyncLatencyCalibrator") << "Lost connection unexpectedly";
			close();
			break;
		}
		if(! bSending){
			lastSendTime = ofGetElapsedTimeMillis();
			ofLogVerbose("ofxNetworkSyncLatencyCalibrator") << "Sending Timestamp: " << lastSendTime;
			string messSend = UDP_MESSAGE_SEND+ofToString(lastSendTime);
			udpSend.Send(messSend.c_str(), messSend.length());
			bSending = true;
			ofSleepMillis(10);
		}else if(lastSendTime+1000 < ofGetElapsedTimeMillis()){
			ofLogNotice("ofxNetworkSyncLatencyCalibrator") << "Timeout!!";
			bSending = false;
		}else{
			udpRecv.Receive(messRecv, messLength);
			string strMessRecv = messRecv;
			if(strMessRecv.length() > 0){
				if(strMessRecv.find(UDP_MESSAGE_REPLY) == 0){
					int recvTime = ofToInt(strMessRecv.substr(UDP_MESSAGE_REPLY.length()));
					ofLogVerbose("ofxNetworkSyncLatencyCalibrator") << "Received Reply: " << strMessRecv;
					if(recvTime > 0 && recvTime == lastSendTime){
						latencies.push_back((ofGetElapsedTimeMillis() - lastSendTime)*0.5);
						ofLogVerbose("ofxNetworkSyncLatencyCalibrator") << "Latency: " << latencies.back();
						bSending = false;
						if(latencies.size() >= numMeasurement){
							ofNotifyEvent(finishMeasuremnt, this);
							close();
						}
					}
				}
			}
		}
	}
}

// ====================================================================================


bool ofxNetworkSyncUdpResponder::setup(string host, int recvPort, int sendPort){
	timeDifferences.clear();
	return ofxNetworkSyncUdpBase::setup(host, recvPort, sendPort);
}
float ofxNetworkSyncUdpResponder::getTimeDifference(){
	if(getTimeDifferenceLogCount() < 2){
		ofLogError("ofxNetworkSyncLatencyCalibrator") << "Data is not enough... " << getTimeDifferenceLogCount();
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
			ofLogError("ofxNetworkSyncLatencyCalibrator") << "Lost connection unexpectedly";
			close();
			break;
		}
		
		udpRecv.Receive(messRecv, 32);
		string strMessRecv = messRecv;
		if(strMessRecv.length() > 0){
			if(strMessRecv.find(UDP_MESSAGE_SEND) == 0){
				int recvTime = ofToInt(strMessRecv.substr(UDP_MESSAGE_SEND.length()));
				ofLogVerbose("ofxNetworkSyncLatencyCalibrator") << "Received Timestamp: " << strMessRecv << ", return it.";
				if(recvTime > 0){
					int now = ofGetElapsedTimeMillis();
					timeDifferences.push_back(now-recvTime);
					ofLogVerbose("ofxNetworkSyncLatencyCalibrator") << "Time difference: " << timeDifferences.back();
					string messSend = UDP_MESSAGE_REPLY+ofToString(recvTime);
					udpSend.Send(messSend.c_str(), messSend.length());
				}
			}
		}
	}
}
