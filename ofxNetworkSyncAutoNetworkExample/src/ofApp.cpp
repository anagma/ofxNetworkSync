#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	ofBackground(0);
	ofSetVerticalSync(true);
	ofSetLogLevel(OF_LOG_VERBOSE);
	
	player.loadSound("sound/1085.mp3");
	player.setLoop(false);

	ofAddListener(finder.serverFound, this, &ofApp::onServerFound);
	if(finder.setup()){
		finderStartTime = 0;
	}else{
		ofLogError("ofApp") << "failed to start finder";
		statusText = "failed to start finder";
		
	}
	statusText = "";
	
	serverPortOffset = 0;
}

//--------------------------------------------------------------
void ofApp::update(){
	unsigned long long now = ofGetElapsedTimeMillis();
	if(!server.isConnected() && !client.isConnected() && !finder.isRunning()){
		if(finder.setup()){
		}else{
			statusText += "\nfailed to start finder";
		}
		
	}
	if(finder.isRunning()){
		if(now > finderStartTime+FINDER_TIMEOUT){
			// server finder timeout

			// i will be server
			if(server.setup(SYNC_TCP_PORT+serverPortOffset)){
				statusText = "i am server";
				player.setPan(1);
				ofRemoveListener(finder.serverFound, this, &ofApp::onServerFound);
				finder.close();
			}else{
				// failed to start server. still try to find server
				statusText = "server failed to start. maybe given address is already in use?";
				finderStartTime = now;
				server.close();
				serverPortOffset++;
			}
		}
	}else if(server.isConnected()){
		server.update();
		
		if(server.hasClients()){
			if (now%SOUND_PLAYER_INTERVAL < lastUpdateTime%SOUND_PLAYER_INTERVAL) {
				bool bPlay = false;
				for (auto & client : server.getClients()) {
					if(client->isCalibrated()){
						bPlay = true;
						client->send("PLAY "+ofToString(now+SOUND_PLAYER_DELAY));
					}
				}
				if(bPlay){
					player.play(SOUND_PLAYER_DELAY);
				}
			}
		}
		
		lastUpdateTime = now;
	}
}
void ofApp::onServerFound(IpAndPort & info){
	statusText = "server found";

	ofRemoveListener(finder.serverFound, this, &ofApp::onServerFound);
	finder.close();
	if(client.setup(info.ip, info.port)){
		player.setPan(-1);
		ofAddListener(client.messageReceived, this, &ofApp::onMessageReceived);
		ofAddListener(client.connectionLost, this, &ofApp::onClientConnectionLost);
	}
}
void ofApp::onMessageReceived(string & message){
	const string messagePlay = "PLAY ";
	if(message.find(messagePlay) == 0 && client.isCalibrated()){
		int time = ofToInt(message.substr(messagePlay.length()));
		player.play(time - client.getSyncedElapsedTimeMillis());
	}
}
void ofApp::onClientConnectionLost(){
	statusText = "client lost connection";
	client.close();
	// retry to find server
	ofAddListener(finder.serverFound, this, &ofApp::onServerFound);
	if(finder.setup()){
		finderStartTime = ofGetElapsedTimeMillis();
	}else{
		statusText += "\nfailed to start finder";
	}
}

//--------------------------------------------------------------
void ofApp::draw(){
	ofSetColor(255);
	ofDrawBitmapString(statusText , 50, 30);
	if(finder.isRunning()){
		ofDrawBitmapString("trying to find server: "+ ofToString((finderStartTime+FINDER_TIMEOUT)-ofGetElapsedTimeMillis()) , 50, 50);
	}else if(client.isConnected()){
		client.drawStatus();
	}else if(server.isConnected()){
		server.drawStatus();
	}
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
