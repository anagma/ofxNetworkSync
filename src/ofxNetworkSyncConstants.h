//
//  ofxNetworkSyncConstants.h
//  toposcan_4screen
//
//  Created by FURUDATE Ken on 1/17/15.
//
//

#ifndef toposcan_4screen_ofxNetworkSyncConstants_h
#define toposcan_4screen_ofxNetworkSyncConstants_h

const int SERVER_SEND_PORT_OFFSET=15000;
const int SERVER_RECV_PORT_OFFSET=17000;


const string MESSAGE_HEADER_CLIENT_ID	= "id";
const string MESSAGE_HEADER_RESULT		= "result";
const string MESSAGE_HEADER_SEPARATOR	= " ";

const string MESSAGE_START_REQUEST		= "calibratin_start";
const string MESSAGE_START_RESPONCE		= "calibratin_start_ok";
const string MESSAGE_START_FAILED		= "calibratin_start_failed";

enum ofxNetworkSyncCalibrationStep{
	WAIT,
	CALIBRATING,
	CALIBRATED
};

const string UDP_MESSAGE_SEND	= "send";
const string UDP_MESSAGE_REPLY	= "reply";

const int NUM_MEASUREMENT_DEFAULT = 10;

#endif
