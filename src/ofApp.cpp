#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    ofSetFrameRate(30);
    
    // set up the kinect
    kinect.init();
    kinect.open();
    kinect.setRegistration(true);
    kinect.setDepthClipping(300,2500);
    
    // zero the tilt on startup
    angle = 0;
    kinect.setCameraTiltAngle(angle);
    
    // initiate the pixels
    image.allocate(640,480,OF_IMAGE_GRAYSCALE);
    
    
    // Gui
    gui.setup();
    gui.setPosition(ofPoint(10,10));
    gui.add(noiseAmount.setup("Noise Amount", 0.0, 0.0,20.0));
    gui.add(pointSkip.setup("Point Skip", 5, 4,20));
    gui.add(minDepth.setup("Min Depth",300,1000,3000));
    gui.add(maxDepth.setup("Max Depth",1000,1500,3000));
    gui.add(cameraZoom.setup("Camera Zoom",1000,400,1500));
    gui.add(useRealColors.setup("Real Colors", true));
    gui.add(showWire.setup("showWire", true));
    gui.add(colorAlpha.setup("Color Alpha", 255,0,255));
    gui.loadFromFile("settings.xml");
    showGui = true;

}

//--------------------------------------------------------------
void ofApp::update(){
    kinect.update();
    if(kinect.isFrameNew()) {
        del.reset();
        
        int w = kinect.getWidth();
        int h = kinect.getHeight();
        
        // pixels for define depth range
        unsigned char* pix = new unsigned char[640*480];
        unsigned char* gpix = new unsigned char[640*480];
        
        
        // set up the depth range
        for(int x = 0; x < w; x++) {
            for(int y = 0; y < h; y++) {
                float distance = kinect.getDistanceAt(x, y);
                
                int pIndex = x + y * 640;
                pix[pIndex] = 0;
                
                if(distance > minDepth && distance < maxDepth) {
                    pix[pIndex] = 255;
                }
                
            }
        }
        
        // make a depth range reference
        image.setFromPixels(pix, 640, 480, OF_IMAGE_GRAYSCALE);
        
        int numPoints = 0;
        
        for(int x=0; x<w; x+=pointSkip*2) {
            for(int y=0; y<h; y+=pointSkip*2) {
                int pIndex = x + 640 * y;
                
                if(image.getPixels()[pIndex]> 0) {
                    ofVec3f wc = kinect.getWorldCoordinateAt(x, y);
                    
                    wc.x = x - 320.0;
                    wc.y = y - 240.0;
                    
                    if(abs(wc.z) > minDepth && abs(wc.z ) < maxDepth) {
                        
                        wc.z = -wc.z;
                        
                        // use perlin noise
                        wc.x += ofSignedNoise(wc.x,wc.z)*noiseAmount;
                        wc.y += ofSignedNoise(wc.y,wc.z)*noiseAmount;
                        
                        wc.x = ofClamp(wc.x, -320,320);
                        wc.y = ofClamp(wc.y, -240,240);
                        
                        del.addPoint(wc);
                    }
                    numPoints++;
                }
                
            }
        }
        
        
        if(numPoints >0)
            del.triangulate();
        
            for(int i=0; i<del.triangleMesh.getNumVertices(); i++) {
                del.triangleMesh.addColor(ofColor(0,0,0));
            }
            
            for(int i=0; i<del.triangleMesh.getNumIndices()/3; i++) {
                ofVec3f v = del.triangleMesh.getVertex(del.triangleMesh.getIndex(i*3));
                
                v.x = ofClamp(v.x, -319,319);
                v.y = ofClamp(v.y, -239, 239);
                
                ofColor c = kinect.getColorAt(v.x+320.0, v.y+240.0);
                
                if(!useRealColors)
                    c = ofColor(255,0,0);
                
                c.a = colorAlpha;
                
                del.triangleMesh.setColor(del.triangleMesh.getIndex(i*3),c);
                del.triangleMesh.setColor(del.triangleMesh.getIndex(i*3+1),c);
                del.triangleMesh.setColor(del.triangleMesh.getIndex(i*3+2),c);
            }
        
        
        
        
            convertedMesh.clear();
            wireframeMesh.clear();
            
            wireframeMesh.setMode(OF_PRIMITIVE_TRIANGLES);
            for(int i=0;i<del.triangleMesh.getNumIndices()/3;i+=1) {
                
                int indx1 = del.triangleMesh.getIndex(i*3);
                ofVec3f p1 = del.triangleMesh.getVertex(indx1);
                int indx2 = del.triangleMesh.getIndex(i*3+1);
                ofVec3f p2 = del.triangleMesh.getVertex(indx2);
                int indx3 = del.triangleMesh.getIndex(i*3+2);
                ofVec3f p3 = del.triangleMesh.getVertex(indx3);
                
                ofVec3f triangleCenter = (p1+p2+p3)/3.0;
                triangleCenter.x += 320;
                triangleCenter.y += 240;
                
                triangleCenter.x = floor(ofClamp(triangleCenter.x, 0,640));
                triangleCenter.y = floor(ofClamp(triangleCenter.y, 0, 480));
                
                int pixIndex = triangleCenter.x + triangleCenter.y * 640;
                if(pix[pixIndex] > 0) {
                    
                    convertedMesh.addVertex(p1);
                    convertedMesh.addColor(del.triangleMesh.getColor(indx1));
                    
                    convertedMesh.addVertex(p2);
                    convertedMesh.addColor(del.triangleMesh.getColor(indx2));
                    
                    convertedMesh.addVertex(p3);
                    convertedMesh.addColor(del.triangleMesh.getColor(indx3));
                    
                    wireframeMesh.addVertex(p1);
                    wireframeMesh.addVertex(p2);
                    wireframeMesh.addVertex(p3);
                }
            }
            
            delete pix;
            delete gpix;
        
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofBackground(0, 0, 0, 0);
    
    ofPushMatrix();
    
    //camera setup
    cam.begin();
    cam.setScale(1,-1,1);
    
    ofTranslate(0, 0, cameraZoom);
    ofFill();
    convertedMesh.drawFaces();
    
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glShadeModel(GL_FLAT);
    glProvokingVertex(GL_FIRST_VERTEX_CONVENTION);
    glShadeModel(GL_SMOOTH);
    glPopAttrib();
    
    if(useRealColors) {
        ofSetColor(30,30,30, 255);
    } else
        ofSetColor(124,136,128,255);
    
    // adjust the position of wireframMesh
    if(showWire){
        ofPushMatrix();
        ofTranslate(0, 0,1);
        ofSetColor(255,255,255);
        wireframeMesh.drawWireframe();
        ofPopMatrix();
    }
    
    cam.end();
    ofPopMatrix();
    
    if(showGui) {
        ofPushStyle();
        ofSetColor(255,255,255,255);
        gui.draw();
        ofPopStyle();
    }
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    switch (key){
        case ' ':
            showGui = !showGui;
            break;
            
        case 'w':
            showWire = !showWire;
            break;
            
        case OF_KEY_UP:
            angle++;
            if(angle>30) angle=30;
            kinect.setCameraTiltAngle(angle);
            break;
            
        case OF_KEY_DOWN:
            angle--;
            if(angle<-30) angle=-30;
            kinect.setCameraTiltAngle(angle);
            break;
            
        case 's':
            
            convertedMesh.save("meshFaces.ply");
            wireframeMesh.save("meshWireFrames.ply");
            break;
            
            
            
            
    }
}

//--------------------------------------------------------------
void ofApp::exit() {
    kinect.close();
    gui.saveToFile("settings.xml");
}