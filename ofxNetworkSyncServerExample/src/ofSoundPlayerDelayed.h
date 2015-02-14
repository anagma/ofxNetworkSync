//
//  syncSoundPlayer.h
//  ofxNetworkSyncServerExample
//
//  Created by FURUDATE Ken on 1/18/15.
//
//

#ifndef ofxNetworkSyncServerExample_ofSoundPlayerDelayed_h
#define ofxNetworkSyncServerExample_ofSoundPlayerDelayed_h

#include "ofMain.h"

class ofSoundPlayerDelayed : public ofSoundPlayer, ofThread{
	int playTime;
	void threadedFunction(){
		int delay = playTime - ofGetElapsedTimeMillis();
		ofSleepMillis(delay);
		play();
	}

public:
	void play(int delay=0){
		if(delay <= 0){
			ofSoundPlayer::play();
		}else{
			playTime = ofGetElapsedTimeMillis()+delay;
			startThread(true);
		}
	}
};

#endif
