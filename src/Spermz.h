#ifndef _SPERMZ
#define _SPERMZ

#include "ofMain.h"
#include "ofCvContourFinder.h"
#include "cv.h"
#define OF_ADDON_USING_OFXOSC
#include "ofAddons.h"

#define MAX_LENGTH_TRACK 200
//#define PI 3.14159265

class Spermz{
    int r,g,b; // color
	float trackPos[MAX_LENGTH_TRACK][2]; // trace
	ofCvBlob blobs[MAX_NUM_CONTOURS_TO_FIND]; // blobs history
	float x,y; // coordinate
	int lostFrames; // number of consecutive frames of lost tracking
	bool justSelected; // flag to know if the track was just selected (A button / mouse click)
	int trackLength; // trace length
	void translatePos(); // translate position by -1 of tracking array
	int index; // index of the array of Spermz
	float direction; // direction in degrees
	float *proximityVector; // distance to other tracks
	int length; //NUM_SPERMZ
public:
	float radius; // radius of capture
	bool active; // flag of active tracking or not
    Spermz(); // constructor
	void desactivateTracking(int i,float r); // desactive tracking of a track
	void displayInfos(ofImage ro,int w,int h,bool f); // display track info (direction/activity) on top
	void update(float rad,float nx,float ny); // update object fields (active tracking)
	void initTrack(int i,int n,float rad,int col); // initialization of tracks
	void computeTrace(int i,ofCvBlob blob); // compute/store trace with contour info
	void computeDirection(int w); // compute direction from last w+1 samples (angle in degrees)
	void drawTrace(int mode,int bmode,float offX,float offY); // draw trace
	void drawBlob(ofCvBlob blob,int mode,bool fMode,float offX,float offY); //draw blob
	void loosingTracking(int nfi,int nf,float r);// check if tracking is getting lost
	void plotTarget(float x,float y); // plot target for selecting sperm
	void sendOSC(ofxOscSender send,float offX,float offY,int w,int h); // send OSC to Pd
	void activateTracking(float nx,float ny); //set object fields when tracking becomes active
	float distance(float nx, float ny); // compute Euclidean distance from current point to (nx,ny)
	void proximityTracks(Spermz s[],int w,int h); // compute distance between active tracks
	void displayConnections(Spermz s[]); // display connection lines between active tracks
};

#endif
	
