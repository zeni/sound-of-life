// Program for "Sound of Life" performances/installations
// by 23N! January 2009
// last updated = 2009.01.26

// Must run DarwiinremoteOSC before to use the wiimote (IR needed)
// Pd must be running to generate sound


#include "ofMain.h"
#include "testApp.h"

//#define DEBUG // comment for live performance/installation (comment in testApp.h too)

//========================================================================
int main( ){
    #ifdef DEBUG
		ofSetupOpenGL(800,600, OF_WINDOW);	// window on main screen
	#else
		ofSetupOpenGL(800,600, OF_FULLSCREEN);	// setup for display on second monitor (fullscreen)
		ofSetWindowPosition(1440,0); // depends on main screen resolution !
	#endif
	ofHideCursor();
	ofRunApp(new testApp());
	
}
