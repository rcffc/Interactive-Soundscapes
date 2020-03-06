#pragma once

#include "ofMain.h"
#include "element.h"

#include <ofxOsc.h>
#include <ofxAbletonLive.h>
#include <boost/circular_buffer.hpp>

class ofApp : public ofBaseApp{

	public:

		ofxOscMessage righthandMessage;
		ofxOscMessage lefthandMessage;
		ofxOscMessage response;
		ofxOscSender sender;
		ofxOscReceiver receiver;	

		ofxAbletonLive live;

		enum { COORD_SIZE = 50, SPEED_SIZE = 50 };

		boost::circular_buffer<Element> RH_coordinates{COORD_SIZE};
		boost::circular_buffer<float> RH_speedDifferences{SPEED_SIZE};

		boost::circular_buffer<Element> LH_coordinates{ COORD_SIZE };
		boost::circular_buffer<float> LH_speedDifferences{ SPEED_SIZE };

		float paramvalue;

		float frequency;
		float phase;
		float phaseInc;
		float amplitude;

		void setup();
		void update();
		void send();
		void exit();
		void draw();

		void updateRighthand();
		void updateLefthand();

		float averageSpeed(boost::circular_buffer<float> diffs);
		void testBuffer();
		
};
