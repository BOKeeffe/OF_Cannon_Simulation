/**
 *
 * Author William O'Keeffe
 * 20050506
 *
 */


#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "ofxGuiExtended.h"

#include "ofxXmlSettings.h"

class ofApp : public ofBaseApp {
    
public:
    
    // game state
    enum GameState {START, PLAY, FIRED, HIT};
    int gameState;
    vector <string> gameStates;
    

    void setup();
    void update();
    void draw();
    
    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void mouseEntered(int x, int y);
    void mouseExited(int x, int y);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);
		
    // simple 3D world with ground and axes
    const float RANGE = 16;
    ofEasyCam easyCam;
    ofPlanePrimitive ground;
    
    // ofxGui code
    ofxPanelExtended gui;
    ofParameterGroup guiParameters;
    ofParameter<bool> isGuiVisible;
    ofParameter<bool> isDebugVisible;
    ofParameter<bool> isAxisVisible;
    ofParameter<bool> isGridVisible;
    ofParameter<bool> isXGridVisible;
    ofParameter<bool> isYGridVisible;
    ofParameter<bool> isZGridVisible;
    ofParameter<bool> isGroundVisible;
    ofParameter<bool> isFullScreen;
    ofParameter<std::string> position;
    
    ofParameter<float> cameraHeightRatio;
    void cameraHeightRatioChanged(float & cameraHeightRatio);

    void reset();
    ofxMinimalButton resetButton;

    ofxMinimalButton quitButton;
    void quitButtonClicked();

    ofTrueTypeFont font;
    
    // simulation specific stuff goes here
    
    // debug code
    ofxTabbedPages debugPages;
    ofxGuiPage debugTextPage, debugGraphicsPage;
    ofxPanelExtended debugGraphicsPanel;

    ofxLabel debugStringLabel;
    string debugString;

    // simulation specific stuff goes here (debug)
    void randomiseTargets();
    void fire();
    void aim();
    
    /** Return the range of a cannon ball for a given elevation. */
    float range(float e) ;
    
    /** Calculate elevation needed to hit at a given distance. */
    float calculateElevation(float TargetDistance);
    
    ofxButton aimButton;
    ofxButton fireButton;
    
   
private:

    // or here

};
