#include "ofApp.h"
#include "element.h"
#include <iostream>
#include <iterator>
#include <math.h>
#include <cmath>

#ifndef NDEBUG
#   define M_Assert(Expr, Msg) \
    __M_Assert(#Expr, Expr, __FILE__, __LINE__, Msg)
#else
#   define M_Assert(Expr, Msg) ;
#endif

void __M_Assert(const char* expr_str, bool expr, const char* file, int line, const char* msg)
{
	if (!expr)
	{
		std::cerr << "Assert failed:\t" << msg << "\n"
			<< "Expected:\t" << expr_str << "\n"
			<< "Source:\t\t" << file << ", line " << line << "\n";
		abort();
	}
}

void ofApp::send() {
	while (true) {
		sender.sendMessage(lefthandMessage, false);
		sender.sendMessage(righthandMessage, false);
		this_thread::sleep_for(2s);
	}
}

float getAverageDistance(boost::circular_buffer<Element> coordinates) {
	float sumDistance = 0.0;

	boost::circular_buffer<Element>::iterator ptr;
	for (ptr = coordinates.begin(); ptr < coordinates.end(); ptr++ ) {
		boost::circular_buffer<Element>::iterator ptr2 = ptr;
		

		float thisX = (ptr->x);
		float thisY = (ptr->y);
		float thisZ = (ptr->z);

		std::advance(ptr2, 1);

		if (ptr2 < coordinates.end()) {
			float nextX = (ptr2->x);
			float nextY = (ptr2->y);
			float nextZ = (ptr2->z);
			sumDistance += sqrt (pow(nextX - thisX, 2) + pow(nextY - thisY, 2) + pow(nextZ - thisZ, 2));
		}
	}

	float averageDistance = sumDistance / coordinates.size();
	return averageDistance;
}

// 0.00000000157 * 1.5^{x}
float fTransform(int index, float value) {
	return 0.0000000000000000075 * pow(2.2, index) * value * 0.5;
}

float ofApp::averageSpeed(boost::circular_buffer<float> speedDifferences) {
	if (speedDifferences.size() == 0) {
		return 0;
	}
	else {
		// float sum = std::accumulate(speedDifferences.begin(), speedDifferences.end(), 0);
		float sum = 0.0;
		int index = 1;

		boost::circular_buffer<float>::reverse_iterator ptr;
		for (ptr = speedDifferences.rbegin(); ptr < speedDifferences.rend(); ptr++) {
			// ofLog() << "index: " << index << " x: " << *ptr << " transform: " << fTransform(index, *ptr);
			sum += fTransform(index, *ptr);
			index++;
		} 
	
		return sum / speedDifferences.size();
	}
}

void printPosition(Element e) {
	ofLog() << e.x << e.y << e.z;
}

//--------------------------------------------------------------
void ofApp::setup(){
	frequency = 440;
	amplitude = 0.1;
	phaseInc = (TWO_PI * frequency) / (float)48000;
	phase = 0;

	lefthandMessage.setAddress("/lefthand_trackjointpos");
	lefthandMessage.addIntArg(1);

	righthandMessage.setAddress("/righthand_trackjointpos");
	righthandMessage.addIntArg(1);

	live.setup();

	sender.setup("127.0.0.1", 12346);
	receiver.setup(12345);


	thread bla(&ofApp::send, this);
	if (bla.joinable()) {
		bla.detach();
	}
}

float calculate128Value(float y) {
	float divider = 128.0 / 800.0; // x -300 900, y -100 700, z -100 700, as approximation I use 100-700
	float paramvalue = ceil((y + 100) * divider); //scaling down
	if (paramvalue < 0.0) {
		paramvalue = 0.0;
	}
	else if (paramvalue > 127.0) {
		paramvalue = 127.0;
	}
	//ofLog() << "x: " << x << " y: " << y << " z: " << z << " value: " << paramvalue;
	return paramvalue;
}

//0.9-2.1
float calculateNoteControlValue(float y) {
	float divider = 1.5 / 800.0; // x -300 900, y -100 700, z -100 700, as approximation I use 100-700
	float paramvalue = 1.5 + y * divider; //scaling down
	if (paramvalue < 0.9) {
		paramvalue = 0.9;
	}
	else if (paramvalue > 2.1) {
		paramvalue = 2.1;
	}
	//ofLog() << "x: " << x << " y: " << y << " z: " << z << " value: " << paramvalue;
	return paramvalue;
}

float calculate1Value(float input) {
	float divider = 1.0 / 100.0; // 50
	float paramvalue = input * divider; //scaling down
	if (paramvalue < 0.0) {
		paramvalue = 0.0;
	}
	else if (paramvalue > 1.0) {
		paramvalue = 1.0;
	}
	// ofLog() << " input: " << input << " value: " << paramvalue;
	return paramvalue;
}

float clip(float p) {
	if (p < 0.0) {
		p= 0.0;
	}
	else if (p > 1.0) {
		p = 1.0;
	}
	return p;
}

void ofApp::updateLefthand() {
	float x = response.getArgAsFloat(0);
	float y = response.getArgAsFloat(1);
	float z = response.getArgAsFloat(2);

	LH_coordinates.push_front(Element("/lefthand_pos_body", x, y, z));
	float avgDistance = getAverageDistance(LH_coordinates);
	LH_speedDifferences.push_front(avgDistance);

	if (live.isLoaded()) {
		ofxAbletonLiveTrack *track = live.getTrack("Left Hand");
		ofxAbletonLiveDevice *device = track->getDevice("Serum_x64");
		ofxAbletonLiveParameter *bWarp = device->getParameter("B Warp");
		ofxAbletonLiveParameter *hyper = device->getParameter("Macro 4");

		float avg = averageSpeed(LH_speedDifferences);
		ofLog() << "LH average speed: " << avg;

		bWarp->setValue(calculate1Value(z));
		hyper->setValue(calculate1Value(y));
		
		track->setVolume(clip(avg));
		live.playContinue();
	}
}


void ofApp::updateRighthand() {

	float x = response.getArgAsFloat(0);
	float y = response.getArgAsFloat(1);
	float z = response.getArgAsFloat(2);

	RH_coordinates.push_front(Element("/righthand_pos_body", x, y, z));
	float avgDistance = getAverageDistance(RH_coordinates);
	RH_speedDifferences.push_front(avgDistance);

	if (live.isLoaded()) {
		ofxAbletonLiveTrack *track = live.getTrack("Right Hand");
		ofxAbletonLiveDevice *device = track->getDevice("Bottle Blower");
		ofxAbletonLiveParameter *fm = device->getParameter("FM Amount");
		ofxAbletonLiveParameter *wavePos = device->getParameter("Wave Position");

		float avg = averageSpeed(RH_speedDifferences);
		ofLog() << "RH average speed: " << avg;

		fm->setValue(calculate128Value(y));
		wavePos->setValue(calculate128Value(z));

		track->setVolume(clip(avg));
		live.playContinue();
	}
}

//--------------------------------------------------------------
void ofApp::update()
{
	live.update();
	do {
		receiver.getNextMessage(&response);
		string address = response.getAddress();
		if (response.getAddress() == "/righthand_pos_body") {
			updateRighthand();
		} else if (response.getAddress() == "/lefthand_pos_body") {
			updateLefthand();
		}
		
	} while (receiver.hasWaitingMessages());
}

//--------------------------------------------------------------
void ofApp::draw() {
	ofSetColor(0);

	if (!live.isLoaded()) {
		ofDrawBitmapString("ofxAbletonLive has not loaded yet!", 100, 100);
		return;
	}

	ofDrawBitmapString(live.getTrackString(), 10, 15);
	int x = 200;

	map<int, ofxAbletonLiveTrack*>::iterator it = live.getTracks().begin();
	for (; it != live.getTracks().end(); ++it)
	{
		string trackString = "Track " + ofToString(it->first) + ": \"" + it->second->getName() + "\"\n";
		trackString += it->second->getDevicesInfo();
		trackString += it->second->getClipsInfo();

		map<int, ofxAbletonLiveDevice*>::iterator itd = it->second->getDevices().begin();
		for (; itd != it->second->getDevices().end(); ++itd) {
			trackString += itd->second->getParametersInfo();
		}

		ofDrawBitmapString(trackString, x, 15);
		x += 300;
	}
}

void ofApp::exit() {
	live.stop();
}