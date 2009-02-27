#include "Spermz.h"

Spermz::Spermz() { // constructor
	r=g=b=200;
	active=false;
	lostFrames=0;
	radius=0;
	trackLength=0;
	justSelected=false;
	direction=0;
 };
 
//--------------------------------------------------------------
void Spermz::initTrack(int i,int n,float rad,int col) {
    radius=rad;
	x=y=-rad;
	index=i;
	length=n;
	proximityVector=new float[n];
	for (int j=0;j<n;j++) proximityVector[j]=-1;
	switch(i) {
		case 0:
			g=b=col;
			break;
		case 1:
			r=b=col;
			break;
		case 2:
			r=g=col;
			break;
		case 3:
			r=col;
			break;
		case 4:
			g=col;
			break;
		case 5:
			b=col;
			break;
		case 6:
			r=g=b=255;
			break;
		case 7:
			r=g=b=col;
			break;
		default:
			r=g=b=0;
			break;
	}
}

//--------------------------------------------------------------
void Spermz::update(float rad,float nx,float ny) {
	justSelected=false;
	radius=rad;
	x=nx;
	y=ny;
	lostFrames=0;
}

//--------------------------------------------------------------
float Spermz::distance(float nx,float ny) {
	return sqrt(pow(nx-x,2)+pow(ny-y,2));
}

//--------------------------------------------------------------
void Spermz::computeTrace(int i,ofCvBlob blob){
	if(i>MAX_LENGTH_TRACK) i=MAX_LENGTH_TRACK;
	trackPos[trackLength][0]=x;
	trackPos[trackLength][1]=y;
	blobs[trackLength]=blob;
	trackLength++;
	if (trackLength>=i) {
		trackLength=i-1;
		translatePos();
	}
}

//--------------------------------------------------------------
void Spermz::computeDirection(int w){
	if (trackLength>1) {
		if(trackLength<w) w=trackLength;
		float dir,mdir=0;
		int ww=0;
		for(int i=1;i<w;i++) { 
			if((trackPos[trackLength-i][1]!=trackPos[trackLength-i-1][1])&(trackPos[trackLength-i][0]!=trackPos[trackLength-i-1][0])) {
				dir=180/PI*atan2((trackPos[trackLength-i][1]-trackPos[trackLength-i-1][1]),trackPos[trackLength-i][0]-trackPos[trackLength-i-1][0]);
				if(dir<0) dir+=360;
				mdir+=dir;	
			}
			else ww++;
		}
		if (ww!=w) direction=mdir/(w-ww);
	}
}

//--------------------------------------------------------------
void Spermz::displayInfos(int w,int h) {
	ofFill();
	if (active) ofSetColor(r,g,b);
	else ofSetColor(r,g,b,50);
	glPushMatrix();
	glTranslatef((index+1)*w/(length+1), h/2+10, 0);
	glRotatef(direction,0,0,1);
	// very simple model !
	ofRect(-h/2,-1,3*h/4,2);
	ofEllipse(3*h/8,0,h/8,3);
	ofEllipse(5*h/16,0,h/16,4);
	glPopMatrix();
	ofNoFill();
}

//--------------------------------------------------------------
void Spermz::drawTrace(int mode,int bmode,float offX,float offY){
	switch(mode) {
		case 0:
			glBegin(GL_LINE_STRIP);
			for (int j = 0; j < trackLength; j++) {
			    ofSetColor(r,g,b,255-255*(trackLength-j)/trackLength);
				glVertex2f(trackPos[j][0], trackPos[j][1]);
			}
			glEnd();
			break;
		case 1:
			for (int j = 0; j < trackLength; j++)
				drawBlob(blobs[j],bmode,true,offX,offY);
			break;
		case 2:
			glBegin(GL_LINE_STRIP);
			for (int j = 0; j < trackLength; j++) {
				ofSetColor(r,g,b,255-255*(trackLength-j)/trackLength);
				glVertex2f(trackPos[j][0], trackPos[j][1]);
			}
			glEnd();
			ofSetColor(r,g,b,100);
			for (int j = 0; j < trackLength; j++)
				drawBlob(blobs[j],bmode,true,offX,offY);
			break;
	}
}

//--------------------------------------------------------------
void Spermz::drawBlob(ofCvBlob blob,int mode,bool fMode,float offX,float offY){
	if(fMode) {
		ofFill();
		ofSetColor(r,g,b,100);
	} else ofSetColor(r,g,b);
	switch(mode) {
		case 0: // contour du blob
		    if(fMode)
				glBegin(GL_POLYGON);
			else 
				glBegin(GL_LINE_LOOP);
			for (int j = 0; j < blob.nPts; j++)
				glVertex2f(blob.pts[j].x+offX, blob.pts[j].y+offY);
			glEnd();
			break;
		case 1: // bounding rect
			ofRect(blob.boundingRect.x+offX, blob.boundingRect.y+offY, blob.boundingRect.width, blob.boundingRect.height);
			break;
		case 2: // bounding ellipse
			ofEllipse(blob.centroid.x+offX, blob.centroid.y+offY, blob.boundingRect.width/2, blob.boundingRect.height/2);
			break;
	}
	if(fMode) ofNoFill();
}

//--------------------------------------------------------------
void Spermz::translatePos(){
	for(int j=0;j<trackLength;j++) {
		trackPos[j][0]=trackPos[j+1][0];
		trackPos[j][1]=trackPos[j+1][1];
		blobs[j]=blobs[j+1];
	}
}

//--------------------------------------------------------------
void Spermz::desactivateTracking(int i,float r){
	if(lostFrames>i) {
		active=false;
		lostFrames=0;
		radius=r;
		trackLength=0;
		justSelected=false;
		direction=0;
		x=y=-r;
		for (int j=index+1;j<length;j++) proximityVector[j]=-1;
	}
}

//--------------------------------------------------------------
void Spermz::loosingTracking(int nfi,int nf,float r){
	lostFrames++;
	translatePos();
	trackLength--;
	if (trackLength<0) trackLength=0;
	if(justSelected) // desactivate tracking quickly if unable to track
		desactivateTracking(nfi,r);
	else
		desactivateTracking(nf,r);
}

//--------------------------------------------------------------
void Spermz::plotTarget(float x,float y) {
	if(active) ofSetColor(r,g,b,100);
	else ofSetColor(r,g,b);
	ofLine(x,y-radius*1.5,x,y-radius*.5);
	ofLine(x,y+radius*1.5,x,y+radius*.5);
	ofLine(x-radius*1.5,y,x-radius*.5,y);
	ofLine(x+radius*1.5,y,x+radius*.5,y);
	ofCircle(x,y,radius);
}

//--------------------------------------------------------------
void Spermz::activateTracking(float nx,float ny){
	x=nx;
	y=ny;
	active=true;
	justSelected=true;
	trackLength=0;
}

//--------------------------------------------------------------
void Spermz::proximityTracks(Spermz s[],float maxdis) {
	for(int j=index+1;j<length;j++) {
		if(!s[j].active) proximityVector[j]=-1;
		else proximityVector[j]=distance(s[j].x,s[j].y)/maxdis; // normalized distance
	}
}

//--------------------------------------------------------------
void Spermz::displayConnections(Spermz s[]) { // change thickness with distance ??
	ofSetColor(r,g,b,50);
	for(int j=index+1;j<length;j++)
		if(s[j].active) {
			ofLine(x,y,s[j].x,s[j].y);
			ofSetColor(s[j].r,(s[j].g,(s[j].b,50);
			ofLine(x,y,s[j].x,s[j].y);
		}
}

//--------------------------------------------------------------
void Spermz::sendOSC(ofxOscSender send,float offX,float offY) {
	ofxOscMessage m;
	stringstream ss;
	string str;
	ss << index;
	ss >> str;
	m.setAddress( "/sz"+str );
	m.addFloatArg(x-offX); // send x
	m.addFloatArg(y-offY); // send y
	m.addFloatArg(direction); // send direction (in degrees)
	// send distance to other tracks
	// -1 means not active track
	// order (example for NUM_SPERMZ=3) is 01 02 03 12 13 23
	for(int j=index+1;j<length;j++) m.addFloatArg(proximityVector[j]);
	send.sendMessage( m );
}

