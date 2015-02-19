#pragma once

#include "ofMain.h"
#include "ofxNetworkSync.h"
#include "ofSoundPlayerDelayed.h"

#define SYNC_TCP_PORT 12345
#define FINDER_TIMEOUT 10000
#define SOUND_PLAYER_DELAY 1000
#define SOUND_PLAYER_INTERVAL 10000

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);

	void onMessageReceived(string & message);
	void onServerFound(IpAndPort & info);
	void onClientConnectionLost();

	ofxNetworkSyncServerFinder finder;
	ofxNetworkSyncClient client;
	ofxNetworkSyncServer server;
	ofSoundPlayerDelayed player;
	
	int lastUpdateTime, finderStartTime;
	
	string statusText;
	int serverPortOffset;
		
};
