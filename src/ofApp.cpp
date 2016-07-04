/*
 *  Author William O'Keeffe
 *  20050506
 *
 */

#include "ofApp.h"
#include "Particle.h"


// cannon attributes
ofParameterGroup cannonParameters;
ofParameter<float> elevation;           ///< rotation about the y-axis
ofParameter<float> direction;           ///< rotation about the z-axis

float muzzleSpeed;                      ///< magnitude of initial velocity
ofVec3f gravity;

//ball for cannon
YAMPE::Particle ball;

//Start Position of Cannon
ofVec3f start(0,0.4f,0) ;

//reset vector for reseting values
ofVec3f resetValues(0,0,0) ;


ofVec3f newTarget ;                    //< target - note y coordinate is zero
bool targetFixed;

//Int To set AutoTargeting mode
int autoTarget = 0;


//--------------------------------------------------------------
void ofApp::setup() {
    
    ofSetVerticalSync(true);
    
    // repatable randomness
    ofSeedRandom(10);

    // create a minimal gui
    gui.setup("3D Application");
    
    cameraHeightRatio.addListener(this, &ofApp::cameraHeightRatioChanged);
    guiParameters.setName("GUI");
    guiParameters.add(isGuiVisible.set("GUI visible", true));
    guiParameters.add(isDebugVisible.set("Debug visible", true));
    guiParameters.add(isAxisVisible.set("Unit vectors", true));
    guiParameters.add(isGridVisible.set("Unit vectors", false));
    guiParameters.add(isXGridVisible.set("Grid (X-axis)", false));
    guiParameters.add(isYGridVisible.set("Grid (Y-axis)", true));
    guiParameters.add(isZGridVisible.set("Grid (Z-axis)", false));
    guiParameters.add(isGroundVisible.set("Ground", true));
    guiParameters.add(cameraHeightRatio.set("Camera (height)", 0.1f, 0., 0.999f));
    
    gui.ofxGuiGroup::add(guiParameters);
    
    // simulation specific stuff goes here

    resetButton.addListener(this, &ofApp::reset);
    gui.add(resetButton.setup("Reset"));

    quitButton.addListener(this, &ofApp::quitButtonClicked);
    gui.add(quitButton.setup("Quit"));
    
    aimButton.addListener(this, &ofApp::aim);
    gui.add(aimButton.setup("Aim"));
    
    fireButton.addListener(this, &ofApp::fire);
    gui.add(fireButton.setup("Fire"));

    // load gui parameters from file
    gui.loadFromFile("settings.xml");
    
    // instantiate the ground
    ground.set(RANGE, RANGE);
    ground.rotate(90, 1,0,0);
    
    // lift camera to 'eye' level
    easyCam.setDistance(RANGE);
    float d = easyCam.getDistance();
    easyCam.setPosition(0, cameraHeightRatio*d, d*sqrt(1.0f-cameraHeightRatio*cameraHeightRatio));
    easyCam.setTarget(ofVec3f::zero());
    
    font.load(OF_TTF_SANS, 9, true, true);

    // build text debug page
    debugTextPage.setup("Text");
    debugString = "";
    debugTextPage.add(&debugStringLabel);
    
    // build text debug page (using a panel to allow for multiple graphs)
    debugGraphicsPage.setup("Graphics");

    debugGraphicsPanel.setup("TODO");
    debugGraphicsPage.add(&debugGraphicsPanel);
    
    // finally build tabbed pages
    debugPages.setup("Debug", "", debugTextPage.getShape().getRight()+10);
    debugPages.setSize(500, 300);
    debugPages.add(&debugTextPage);
    debugPages.add(&debugGraphicsPage);
    
    // set position of debug GUI to bottom left corner
    debugPages.setPosition(0, ofGetHeight()-debugPages.getHeight());
    
    // initialise gameStates
    string gameStateLabels[] = {"START", "PLAY", "FIRED", "HIT"};
    gameStates.assign(gameStateLabels, gameStateLabels+4);
    gameState = START;

    //Set target to false and playState to Start
    targetFixed = false ;
    gameState = START ;
    
    
    //set vec gravity
    gravity.set(0.0f,-0.981f, 0.0f);
    
    muzzleSpeed = 4;
    
    cannonParameters.setName("Cannon");
    cannonParameters.add(elevation.set("Elevation", 0, 0, 90));
    cannonParameters.add(direction.set("Direction", 0, 0, 360));
    gui.ofxGuiGroup::add(cannonParameters);
    
}



void ofApp::reset() {
    gameState = START;
}

void ofApp::update() {

    float dt = ofClamp(ofGetLastFrameTime(), 0.0, 0.02);
    
    //switch for State machine
    switch (gameState) {
        case START:{
            //Start case to set random target and change to play case
            if(!targetFixed)
            {
                randomiseTargets() ;
            }
            //ball position set to start position and velocity and gravity are reset to zero
            ball.setPosition(start) ;
            ball.setVelocity(resetValues) ;
            ball.acceleration = resetValues ;
            gameState = PLAY ;
            break;
        }
        case PLAY:{
            
            if(autoTarget == 1 && targetFixed)
            {
                aim() ;
                fire() ;
            }
            break;
        }
        case FIRED:{
            //Integrate called and draw function
            ball.integrate(dt) ;
            ball.draw() ;
            ball.toString() ;
            
            //check ball position on the y axis
            if(ball.position.y < 0.1f)
            {
                //Checkif ball collided with the target) change gameState
                if(ball.position.distance(newTarget) <= 1)
                    gameState = HIT;
                else//Keep playing(firing)
                    gameState = PLAY;
            }
            break;
        }
        case HIT:{
            targetFixed = false;
            gameState = START ;
            break;
        }
        default:
            break;
    }

}




void ofApp::draw() {
    
    ofEnableDepthTest();
    ofBackgroundGradient(ofColor(128), ofColor(0), OF_GRADIENT_BAR);
    
    ofPushStyle();
    easyCam.begin();
    
    ofDrawGrid(RANGE/(2*8), 8, false, isXGridVisible, isYGridVisible, isZGridVisible);
    
    if (isAxisVisible)
            ofDrawAxis(1);
    
    if (isGroundVisible) {
        ofPushStyle();
        ofSetHexColor(0xB87333);
        ground.draw();
        ofPopStyle();
    }
    
    //Draw cannon base
    ofSetColor(255,0,0) ;
    ofDrawBox(0,0.2f,0,0.8f,0.4f,0.8f) ;
    ofSetColor(200,0,0) ;
    ofDrawBox(0,0.2f,0,0.8f,0.4f,0.8f) ;
    //ball on top of base
    ofFill();
    ofDrawSphere(0,0.4f,0,0.3f) ;
    
    //Draw cannon turret
    glPushMatrix() ;
    ofSetColor(0,255,200) ;
    ofTranslate(0,0.4f,0) ;
    ofRotate(direction, 0.0f, 1.0f, 0.0f) ;
    ofRotate(-elevation+90, 1.0f, 0.0f, 0.0f) ;
    ofTranslate(0,-0.4f,0) ;
    ofDrawCylinder(0,0.9f,0,0.15f,1) ;
    glPopMatrix() ;
    
    //Draw new target using random coordinates of newTarget
    ofSetColor(0, 0, 255);
    ofDrawSphere(newTarget, 0.5f) ;
    
    
    // simulation specific stuff goes here
    ofApp::update();
    
    ofPushStyle() ;

    easyCam.end();
    ofPopStyle();

    //position.set(ofToString((int)ofGetFrameRate()));
    
    // finally render any GUI/HUD
    ofDisableDepthTest();
    if (isGuiVisible) gui.draw();
    if (isDebugVisible) {
        debugString = "Game State = " + ofToString(gameState) + "\n\n"
                    + "Ball  Position             = " + ofToString(ball.position)
            + "\n" +  "      Velocity             = " + ofToString(ball.velocity)
            + "\n" +  "      Accerleration        = " + ofToString(ball.acceleration)
            + "\n" +  "      Distance to target   = " + " ToDo...... " ;
        
        debugStringLabel.setup("",
           "FPS = " + ofToString((int)ofGetFrameRate()) + "\n\n" + debugString);

        debugPages.draw();
        
    }
    
    // Display some useful info
    ofSetColor(255, 0, 0);
    ofDrawBitmapString(ofToString ((int) ofGetFrameRate ()) + " fps ", 500, 20);
    ofDrawBitmapString(" Elevation = " + ofToString (elevation), 500, 40);
    ofDrawBitmapString(" Direction = " + ofToString (direction), 500, 60);
    ofDrawBitmapString(" Game State = " + ofToString (gameState), 500, 80);
}



//--------------------------------------------------------------
// GUI events and listeners
//--------------------------------------------------------------

void ofApp::keyPressed(int key) {
    
    float d = easyCam.getDistance();
    
    switch (key) {
        
        case 'h':                               // toggle GUI/HUD
            isGuiVisible = !isGuiVisible;
            break;
        case 'b':                               // toggle debug
            isDebugVisible = !isDebugVisible;
            break;
        case 't':                               // toggle debug
            isDebugVisible = !isDebugVisible;
            break;
        case 'a':                               // toggle axis unit vectors
            isAxisVisible = !isAxisVisible;
            break;
        case '1':                               // toggle grids (X)
            isXGridVisible = !isXGridVisible;
            break;
        case '2':                               // toggle grids (Y)
            isYGridVisible = !isYGridVisible;
            break;
        case '3':                               // toggle grids (Z)
            isZGridVisible = !isZGridVisible;
            break;
        case 'g':                               // toggle ground
            isGroundVisible = !isGroundVisible;
            break;
        case 'u':                               // set the up vecetor to be up (ground to be level)
            easyCam.setTarget(ofVec3f::zero());
            break;

        case 'S' :                              // save gui parameters to file
            gui.saveToFile("settings.xml");
            
            break;
        case 'L' :                              // load gui parameters
            gui.loadFromFile("settings.xml");
            break;

        case 'z':
            easyCam.setPosition(0, cameraHeightRatio*d,
                                d*sqrt(1.0f-cameraHeightRatio*cameraHeightRatio));
            easyCam.setTarget(ofVec3f::zero());
            break;
        case 'Z':
            easyCam.setPosition(0, 0, d);
            easyCam.setTarget(ofVec3f::zero());
            break;
        case 'x':
            easyCam.setPosition(d*sqrt(1.0f-cameraHeightRatio*cameraHeightRatio), cameraHeightRatio*d, 0);
            easyCam.setTarget(ofVec3f::zero());
            break;
        case 'X':
            easyCam.setPosition(d, 0, 0);
            easyCam.setTarget(ofVec3f::zero());
            break;
        case 'Y':
            easyCam.setPosition(0.001, d, 0);
            easyCam.setTarget(ofVec3f::zero());
            break;
        case 'm':
            //set aim to tartget
            gameState = PLAY;
            aim();
            break;
            
        case 'f':                               // toggle fullscreen
            // BUG: window size is not preserved
            isFullScreen = !isFullScreen;
            if (isFullScreen) {
                ofSetFullscreen(false);
            } else {
                ofSetFullscreen(true);
            }
        case ' ':
            if(gameState == PLAY){
                fire();
            }
            break;
            
         case OF_KEY_LEFT:
            direction -= 1;
            if(direction == 0){
                direction = 360;
            }
            break;
            
        case OF_KEY_RIGHT:
            direction += 1;
            if(direction == 360){
                direction = 0;
            }
            break;
         
        case OF_KEY_UP:

            if(elevation <= 90){
                elevation +=1;
            }
            break;
            
        case OF_KEY_DOWN:
            if(elevation >= 0){
                elevation -= 1;
            }
            
            
        // simulation specific stuff goes here

    }
}

void ofApp::fire(){
    //TODO
    //Create base Vector
    ofVec3f base = ofVec3f(0,1,0) ;
    
    ofMatrix4x4 matrix ;
    matrix.makeRotationMatrix(-elevation+90, ofVec3f(1,0,0), direction, ofVec3f(0,1,0), 0, ofVec3f(0,0,1)*matrix) ;
    
    //multiply unit vector by rotation matrix
    ofVec3f direction = base * matrix ;
    
    ball.setPosition(start) ;
    ball.set
    ball.setVelocity(direction*muzzleSpeed) ;
    ball.acceleration = gravity ;
    gameState = FIRED ;
    
}

void ofApp::aim(){
    
    float value = 180/3.1459265;
    //calculate Direction using standard c atan2f
    direction = atan2f(newTarget.x, newTarget.z) * value;
    
    //Keeps direction between 0 and 360.
    if(direction < 0) direction =  360+direction ;
    
    
    //calculate Elevation
    elevation = calculateElevation(ball.position.distance(newTarget));
    
}

//Code taken from calss lectures
float ofApp::range(float e)
{
    float ux = muzzleSpeed * cos(ofDegToRad(e));
    float uy = muzzleSpeed * sin(ofDegToRad(e));
    float h = 0.02f ;
    float g = -gravity.y;
    
    return (ux/g) * (uy + sqrt(uy*uy + 2*g*h));
}

/** Calculate elevation needed to hit at a given distance.
  *  Code taken from class lectures
 */
float ofApp::calculateElevation(float targetDistance){
    
    float xMin(0.0f), fxMin(targetDistance - range(xMin));
    float xMax(90.0f), fxMax(targetDistance - range(xMax));
    
    while(xMax - xMin > 0.1e-3)
    {
        float x = 0.5*(xMin+xMax);
        float fx = targetDistance - range(x);
        
        ofLog(OF_LOG_NOTICE, "%f .. %f .. %f => %f .. %f .. %f\n",
              xMin, x, xMax, fxMin, fx, fxMax);
        
        if(fxMin*fx < 0)
        {
            xMax = x;
            fxMax = fx;
        }
        else
        {
            xMin = x;
            fxMin = fx;
        }
        return 0.5*(xMin+xMax);
    }
   
}


void ofApp::cameraHeightRatioChanged(float & cameraHeightRatio) {
    
    float d = easyCam.getDistance();
    easyCam.setPosition(0, cameraHeightRatio*d, d*sqrt(1.0f-cameraHeightRatio*cameraHeightRatio));
    easyCam.setTarget(ofVec3f::zero());
}


void ofApp::quitButtonClicked() {
    ofExit();
}

//set new randomised position for a new target
void ofApp::randomiseTargets()
{
    // set a newTarget with Randomised x,y position between -9 and 9
    newTarget.set(ofRandom(-9,9),0,ofRandom(-9,9));
}

//--------------------------------------------------------------
// Unused
//--------------------------------------------------------------
void ofApp::keyReleased(int key) {}
void ofApp::mouseMoved(int x, int y ) {}
void ofApp::mouseDragged(int x, int y, int button) {}
void ofApp::mousePressed(int x, int y, int button) {
    easyCam.enableMouseInput();
}
void ofApp::mouseReleased(int x, int y, int button) {}
void ofApp::mouseEntered(int x, int y) {}
void ofApp::mouseExited(int x, int y) {}
void ofApp::windowResized(int w, int h) {}
void ofApp::gotMessage(ofMessage msg) {}
void ofApp::dragEvent(ofDragInfo dragInfo) {}
