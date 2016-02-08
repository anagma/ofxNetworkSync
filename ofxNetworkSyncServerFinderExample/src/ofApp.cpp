#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	
	ofBackground(0);
	ofSetVerticalSync(true);
	ofSetLogLevel(OF_LOG_VERBOSE);
	
	player.load("sound/1085.mp3");
	player.setPan(-1);
	player.setLoop(false);

	finder.setup();
}

//--------------------------------------------------------------
void ofApp::update(){
	if(finder.isRunning() && finder.doesServerFound()){
		finder.close();
		if(client.setup(finder.getServerInfo().ip, finder.getServerInfo().port)){
			ofAddListener(client.messageReceived, this, &ofApp::onMessageReceived);
		}else{
			ofLogError() << "failed to start client";
		}
	}
}
void ofApp::onMessageReceived(string & message){
	const string messagePlay = "PLAY ";
	if(message.find(messagePlay) == 0 && client.isCalibrated()){
		int time = ofToInt(message.substr(messagePlay.length()));
		player.play(time - client.getSyncedElapsedTimeMillis());
	}
}

//--------------------------------------------------------------
void ofApp::draw(){
	client.drawStatus();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
