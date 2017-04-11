#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "ofxKinect.h"
#include "ofxDelaunay.h"

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
        void exit();
    
    //kinect
    //http://openframeworks.cc/documentation/ofxKinect/
    ofxKinect kinect;
    
    //triangulation based on Delaunay
    //https://github.com/obviousjim/ofxDelaunay
    ofxDelaunay del;
    
    // gui
    bool showGui;
    ofxPanel gui;
    ofxIntSlider colorAlpha;
    ofxFloatSlider noiseAmount;
    ofxToggle useRealColors;
    ofxToggle showWire;
    ofxIntSlider pointSkip;
    ofxIntSlider minDepth;
    ofxIntSlider maxDepth;
    ofxIntSlider cameraZoom;
    
    // meshes
    ofMesh convertedMesh;
    ofMesh wireframeMesh;
    ofImage image;
    
    // kinect angle
    int angle;
    
    //3d camera
    ofEasyCam cam;
		
};
