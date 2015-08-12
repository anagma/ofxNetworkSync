#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	ofBackground(0);
	ofSetVerticalSync(true);
	ofSetLogLevel(OF_LOG_VERBOSE);
	
	player.loadSound("sound/1085.mp3");
	player.setLoop(false);

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
		if(finder.doesServerFound()){
			statusText = "server found";
			
			finder.close();
			if(client.setup(finder.getServerInfo().ip, finder.getServerInfo().port)){
				player.setPan(-1);
				ofAddListener(client.messageReceived, this, &ofApp::onMessageReceived);
				ofAddListener(client.connectionLost, this, &ofApp::onClientConnectionLost);
			}
			
		}
		
		if(now > finderStartTime+FINDER_TIMEOUT){
			// server finder timeout

			// i will be server
			if(server.setup(SYNC_TCP_PORT+serverPortOffset)){
				statusText = "i am server";
				player.setPan(1);
//				ofRemoveListener(finder.serverFound, this, &ofApp::onServerFound);
				finder.close();
				
				if(client.isCalibrated()){
					server.setTimeOffsetMillis(client.getSyncedElapsedTimeMillis()-ofGetElapsedTimeMillis());
				}
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
			if (server.getSyncedElapsedTimeMillis()%SOUND_PLAYER_INTERVAL < lastUpdateTime%SOUND_PLAYER_INTERVAL) {
				bool bPlay = false;
				for (auto & client : server.getClients()) {
					if(client->isCalibrated()){
						bPlay = true;
						client->send("PLAY "+ofToString(server.getSyncedElapsedTimeMillis()+SOUND_PLAYER_DELAY));
					}
				}
				if(bPlay){
					player.play(SOUND_PLAYER_DELAY);
				}
			}
		}
		
		lastUpdateTime = server.getSyncedElapsedTimeMillis();
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
		ofDrawBitmapString(ofToString(client.getSyncedElapsedTimeMillis()), 50, ofGetHeight()-70);
	}else if(client.isConnected()){
		client.drawStatus();
		ofDrawBitmapString(ofToString(client.getSyncedElapsedTimeMillis()), 50, ofGetHeight()-70);
	}else if(server.isConnected()){
		server.drawStatus();
		ofDrawBitmapString(ofToString(server.getSyncedElapsedTimeMillis()), 50, ofGetHeight()-70);
	}
	
//	if(client.isConnected()){
//	}else if(server.isConnected()){
//		ofDrawBitmapString(ofToString(server.getSyncedElapsedTimeMillis()), 200, 30);
//	}
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}