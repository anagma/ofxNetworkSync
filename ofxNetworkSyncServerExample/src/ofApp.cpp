#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	ofBackground(0);
	ofSetVerticalSync(true);
	ofSetLogLevel(OF_LOG_VERBOSE);
	
	if(server.setup(12345)){
		player.loadSound("sound/1085.mp3");
		player.setPan(1);
		player.setLoop(false);
	}

}

//--------------------------------------------------------------
void ofApp::update(){
	if(server.isConnected()){
		server.update();
		
		if (ofGetElapsedTimeMillis()%10000 < lastUpdateTime%10000) {
			if(server.hasClients()){
				bool bPlay = false;
				for (auto & client : server.getClients()) {
					if(client->isCalibrated()){
						bPlay = true;
						client->send("PLAY "+ofToString(ofGetElapsedTimeMillis()+1000));
					}
				}
				if(bPlay){
					player.play(1000);
				}
			}
		}
		
		lastUpdateTime = ofGetElapsedTimeMillis();
	}
}

//--------------------------------------------------------------
void ofApp::draw(){
	server.drawStatus();
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
