#ifndef _TEST_APP
#define _TEST_APP

#include "ofMain.h"
#include "ofCvMain.h"
#define OF_ADDON_USING_OFXOSC
#include "ofAddons.h"
#include "Spermz.h"

//receiving port (from darwiinmote)
#define PORT 5600
//sending port (to Pd)
#define HOST "localhost"
#define PORTS 9997
// Number of tracks
#define NUM_SPERMZ 3
//size of movie
#define MOVIE_HEIGHT 480
#define MOVIE_WIDTH 640
// length of trace (!Careful! Do not exceed the size of the array in the class) 
#define TRACE_LENGTH 99
// uncomment for using camera
//#define LIVE_VIDEO 
 // comment for not showing mouse target
#define USE_MOUSE
// comment for live performance (comment in main.cpp) too
#define DEBUG

class testApp : public ofSimpleApp{
	public:
		void setup();
		void update();
		void draw();
		void exit();
		void receiveOSC();
		void keyPressed (int key);
		void mouseMoved(int x, int y );
		void mousePressed(int x, int y, int button);
		
		#ifdef LIVE_VIDEO
			ofVideoGrabber 		spermMovie;
		#else
			ofVideoPlayer 		spermMovie;
		#endif
		ofCvGrayscaleImage 	grayDiff;
		ofCvColorImage		currFrameColor;
		ofCvGrayscaleImage  currFrame;
		ofCvGrayscaleImage 	prevFrame;

		ofCvContourFinder 	contourFinder;
		
	private:
		//ofCvContourFinder 	blobsHistory[NUM_SPERMZ][TRACE_LENGTH];
		int threshold; // threshold for contour detection
		int selectedSpermz; // current selected track
		float radius,initRadius; // radius of capture and initial radius of capture (should be larger)
		float irX,irY,irSize; // IR position and spot size (wiimote)
		float mouseX,mouseY; // mouse position
		Spermz * sperm;
		int minArea,maxArea; // area of contours
		int windowLength; // window fo mean computation on trace(direction,...)
		int mode; // display mode of blobs
		int modeTrace; // display mode of trace
		bool fillMode; // fill or not contours of blobs
		int numFrames,numFramesInit; // Number of frames before desactiving tracking, initial (just selected case)
		ofxOscReceiver	receiver;
		ofxOscSender sender;
		float posX,posY; // offset of movie
		bool trace; // plot trace of tracking or not
		bool connect; // plot connection lines or not
};

#endif
