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
	int delay;
	void threadedFunction(){
		ofSleepMillis(delay);
		play();
	}
	
public:
	void play(int delay_=0){
		if(delay_ <= 0){
			ofSoundPlayer::play();
		}else{
			ofLogVerbose("ofSoundPlayerDelayed") << "play sound after " << delay << " ms.";
			delay = delay_;
			startThread(true);
		}
	}
};

#endif
