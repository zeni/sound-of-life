#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){	 
	// camera or movie
#ifdef LIVE_VIDEO
	spermMovie.setVerbose(true);
	spermMovie.initGrabber(MOVIE_WIDTH,MOVIE_HEIGHT);
	//spermMovie.listDevices();
#else
	spermMovie.loadMovie("movies/macam movie 2.mov");
	spermMovie.play();
#endif
	// frames
	currFrame.allocate(MOVIE_WIDTH,MOVIE_HEIGHT); // current frame grayscale
	currFrameColor.allocate(MOVIE_WIDTH,MOVIE_HEIGHT); // current frame not processed
	prevFrame.allocate(MOVIE_WIDTH,MOVIE_HEIGHT); // previous frame grayscale
	grayDiff.allocate(MOVIE_WIDTH,MOVIE_HEIGHT); // difference of frames (grayscale)
	// some initializations
	minArea=3;
	maxArea=200;
	threshold = 4;
	selectedSpermz=0;
	initRadius=30;
	radius=20;
	mode=modeTrace=0;
	numFrames=40;
	numFramesInit=15;
	trace=true;
	fillMode=true;
	connect=false;
	irX=irY=mouseX=mouseY=-100;
	windowLength=5;
	sperm = new Spermz[NUM_SPERMZ];
	for (int i=0;i<NUM_SPERMZ;i++)
		sperm[i].initTrack(i,NUM_SPERMZ,initRadius,0); // color and some initializations !
	// OSC settings
	receiver.setup(PORT);
	sender.setup(HOST, PORTS);
	// other settings
	ofNoFill();
	ofEnableAlphaBlending();
#ifdef DEBUG
	posX=0;posY=0;
#else
	posX=(ofGetWidth()-MOVIE_WIDTH)/2;posY=(ofGetHeight()-MOVIE_HEIGHT)/2; // for dual screen
#endif
}

//--------------------------------------------------------------
void testApp::update(){
	ofBackground(100,100,100);
	// camera or movie
#ifdef LIVE_VIDEO
	spermMovie.grabFrame();
#else
	spermMovie.idleMovie();
#endif
	// contour detection
	if (spermMovie.isFrameNew()){
		prevFrame=currFrame;
		currFrameColor.setFromPixels(spermMovie.getPixels(),MOVIE_WIDTH,MOVIE_HEIGHT);
		currFrame.setFromColorImage(currFrameColor);
		grayDiff.Sub(prevFrame, currFrame);
		grayDiff.threshold(threshold);
		// contour detection: (image,min area,max area,max number of contours, hole detection)
		contourFinder.findContours(grayDiff,minArea,maxArea,50,false);
	}
	// reception of data from wiimote (OSC)
	receiveOSC();
}

//--------------------------------------------------------------
void testApp::draw(){
	// display movie/camera
#ifdef DEBUG
	currFrame.draw(posX,posY);
	grayDiff.draw(posX+spermMovie.width+10,posY);
#else
	currFrame.draw(posX,posY);
#endif
	// Target (=wiimote cursor or mouse) for selecting spermatozoa of the color of the current track
	sperm[selectedSpermz].plotTarget(irX,irY);
#ifdef USE_MOUSE
	sperm[selectedSpermz].plotTarget(mouseX,mouseY);
#endif
	// Associate a spermatozoid (and only one) with a track
	// Try to track always the same spermatozoide for a given track
	for (int j=0;j<NUM_SPERMZ;j++) {
	    sperm[j].displayInfos(800,50);
		bool foundBlobs=false;
		float distance=1000; // arbitrary large distance
		float distanceCurr;
		float centre[2];
		int contourIndex=0;
		if(sperm[j].active) {
		    // look for the closest blob
			for (int i = 0; i < contourFinder.nBlobs; i++){
				centre[0]=contourFinder.blobs[i].centroid.x+posX;
				centre[1]=contourFinder.blobs[i].centroid.y+posY;
				distanceCurr=sperm[j].distance(centre[0],centre[1]);
				if(distanceCurr<sperm[j].radius) {
					foundBlobs=true;
					if(distanceCurr<distance) {
						contourIndex=i;
						distance=distanceCurr;
					}
				}
			}
			if (trace) sperm[j].drawTrace(modeTrace,mode,posX,posY);
			// manage tracking
			if(!foundBlobs) {
				sperm[j].loosingTracking(numFramesInit,numFrames,initRadius);
			} else {
				sperm[j].drawBlob(contourFinder.blobs[contourIndex],mode,fillMode,posX,posY);
				sperm[j].update(radius,contourFinder.blobs[contourIndex].centroid.x+posX,contourFinder.blobs[contourIndex].centroid.y+posY);
				sperm[j].computeTrace(TRACE_LENGTH,contourFinder.blobs[contourIndex]); // needed for computing direction
				sperm[j].computeDirection(windowLength);
			}
			if(connect) sperm[j].displayConnections(sperm);
		}
	}
	for (int j=0;j<NUM_SPERMZ;j++)
		if(sperm[j].active) {
			sperm[j].proximityTracks(sperm,sqrt(pow(spermMovie.width,2)+pow(spermMovie.height,2)));
			sperm[j].sendOSC(sender,posX,posY);
		}
	ofSetColor(0xffffff);
#ifdef DEBUG
	char reportStr[1024];
	sprintf(reportStr, "threshold %i (press: +/-)\nnum blobs found %i\nIR X and Y : %f, %f (size: %f)", threshold, contourFinder.nBlobs,irX,irY,irSize);
	ofDrawBitmapString(reportStr, 20, 500);
	sprintf(reportStr, "min area %i (press: q/a)\nmax area %i (press:w/s)\n", minArea, maxArea);
	ofDrawBitmapString(reportStr, 20, 540);
	sprintf(reportStr, "Frames (lost tracking): %i (press: </>)", numFrames);
	ofDrawBitmapString(reportStr, 20, 570);
#endif
}

//--------------------------------------------------------------
void testApp::receiveOSC(){
	while( receiver.hasWaitingMessages() )
	{
		// get the next message
		ofxOscMessage m;
		receiver.getNextMessage( &m );
		// IR spot
		if (  m.getAddress() =="/wii/irdata") {
			irX=(1-m.getArgAsFloat( 0 ))*spermMovie.width+posX;
			irY=m.getArgAsFloat( 1 )*spermMovie.height*4/3+posY;
			irSize=m.getArgAsFloat( 2 );
		}
		// Activate tracking
		if (  m.getAddress() =="/wii/button/a") {
			if (m.getArgAsInt32( 0 )==1) {
				sperm[selectedSpermz].activateTracking(irX,irY);
				selectedSpermz++;
				if (selectedSpermz==NUM_SPERMZ) selectedSpermz=0;
			}
		}
		// Change track
		if (  m.getAddress() =="/wii/button/b") {
			if (m.getArgAsInt32( 0 )==1) {
				selectedSpermz++;
				if (selectedSpermz==NUM_SPERMZ) selectedSpermz=0;
			}
		}
	}
}

//--------------------------------------------------------------
void testApp::keyPressed  (int key){ 
	switch (key){
		case 's': // camera settings
#ifdef LIVE_VIDEO
			spermMovie.videoSettings();
#endif
			break;
		case 'b': // simulate B button of wiimote
			selectedSpermz++;
			if (selectedSpermz==NUM_SPERMZ) selectedSpermz=0;
			break;
		case '+': // threshold +
			threshold++;
			if (threshold > 255) threshold = 255;
			break;
		case '-': // threshold -
			threshold--;
			if (threshold < 0) threshold = 0;
			break;
		case 'q': // min area +
			minArea++;
			if (minArea >= maxArea) minArea--;
			break;
		case 'a': // min area -
			minArea--;
			if (minArea < 1) minArea = 1;
			break;
		/*case 'w': // max area +
			maxArea++;
			break;
		case 's': // max area -
			maxArea --;
			if (maxArea <= minArea) maxArea++;
			break;*/
		case 'm': // contour display mode
			mode++;
			if (mode > 2) mode=0;
			break;
		case 'n': // trace display mode
			modeTrace++;
			if (modeTrace > 2) modeTrace=0;
			break;
		case 'f': // contour fill switch
			fillMode=!fillMode;
			break;
		case '>': // num frame tracking lost +
			numFrames++;
			break;
		case '<': // num frame tracking lost -
			numFrames--;
			if (numFrames < 1) numFrames++;
			break;	
		case 't': // display or not trace of tracking
			trace=!trace;
			break;		
		case 'c': // display or not connection lines
			connect=!connect;
			break;	
	}
}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){
	mouseX=(float) x;
	if (mouseX<posX) mouseX=posX;
	if (mouseX>spermMovie.width+posX) mouseX=spermMovie.width+posX;
	mouseY=(float) y;
	if (mouseY<posY) mouseY=posY;
	if (mouseY>spermMovie.height+posY) mouseY=spermMovie.height+posY;
}	

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){ // in case no wiimote is available 
	sperm[selectedSpermz].activateTracking((float) x,(float) y);
	selectedSpermz++;
	if (selectedSpermz==NUM_SPERMZ) selectedSpermz=0;
}

//--------------------------------------------------------------
void testApp::exit() {
#ifdef LIVE_VIDEO
	spermMovie.close();
#else
	spermMovie.closeMovie();
#endif
}