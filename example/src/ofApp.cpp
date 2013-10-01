// =============================================================================
//
// Copyright (c) 2010-2013 Christopher Baker <http://christopherbaker.net>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// =============================================================================


#include "ofApp.h"


//------------------------------------------------------------------------------
void ofApp::setup()
{
    ofSetLogLevel(OF_LOG_VERBOSE);
    ofSetFrameRate(120);
    ofEnableAlphaBlending();
    ofEnableSmoothing();
    
    TouchPadRef pad = TouchPad::getTouchPadRef();

    pad.setScalingRect(ofRectangle(100,100,160*5,120*5));
    pad.setScalingMode(TouchPad::SCALE_TO_RECT);
    
    ofAddListener(ofEvents().touchUp,        this, &ofApp::touchUp);
    ofAddListener(ofEvents().touchDoubleTap, this, &ofApp::touchDoubleTap);
    ofAddListener(ofEvents().touchDown,      this, &ofApp::touchDown);
    ofAddListener(ofEvents().touchMoved,     this, &ofApp::touchMoved);

    //  The following code attempts to prevent conflicts between system-wide
    //  gesture support and the raw TouchPad data provided by ofxTouchPad.
    //    ofSystem("killall -STOP Dock"); // turn off OS level gesture support ...
    //    CGAssociateMouseAndMouseCursorPosition(false);
    //    ofHideCursor();
}

//------------------------------------------------------------------------------
void ofApp::exit()
{
    //  The following code re-enables default system-wide gesture support.
    //    ofSystem("killall -CONT Dock"); // turn on OS level gesture support
    //    CGAssociateMouseAndMouseCursorPosition(true);
    //    ofShowCursor();
}

//------------------------------------------------------------------------------
void ofApp::draw()
{
    ofBackground(0);
    
    TouchPadRef pad = TouchPad::getTouchPadRef();

    ofSetColor(255,255,255);
    ofDrawBitmapString("TouchCount: " + ofToString(pad.getTouchCount(), 0), 20, 20);
    
    // Simple
    ofFill();
    ofSetColor(255, 255, 255, 100);

    ofRectangle scalingRect = pad.getScalingRect();
    
    ofRectRounded(scalingRect, 10);
    
    TouchPad::Touches touches = pad.getTouches();

    for(std::size_t i=0; i<touches.size(); ++i)
    {
        ofPushMatrix();
        ofTranslate(touches[i].x, touches[i].y);
        
        float w = touches[i].majoraxis * 5;
        float h = touches[i].minoraxis * 5;
        
        float pressure = touches[i].pressure * 20;
        
        ofRotateZ(ofRadToDeg(touches[i].angle));
        ofSetColor(255,100);
        ofEllipse(0,0,w,h);
        ofSetColor(255,255,0,100);
        ofEllipse(0,0,pressure,pressure);
        ofSetColor(255,100);
        ofLine(-w/2.0f,0.0f,w/2.0f,0.0f);
        ofLine(0.0f,-h/2.0f,0.0f,h/2.0f);
        ofPopMatrix();
        
        ofFill();
        ofSetColor(255);
        ofDrawBitmapString(ofToString(touches[i].id),touches[i].x-6, touches[i].y+3);
        ofSetColor(255, 255, 255, 100);

    }
}

//------------------------------------------------------------------------------
std::string ofApp::touchToString(const ofTouchEventArgs& touch)
{
    std::stringstream ss;
    
    std::string typeString;
    switch(touch.type) {
        case ofTouchEventArgs::down:
            typeString = "DOWN";
            break;
        case ofTouchEventArgs::up:
            typeString = "UP";
            break;
        case ofTouchEventArgs::move:
            typeString = "MOVE";
            break;
        case ofTouchEventArgs::doubleTap:
            typeString = "DOUBLE TAP";
            break;
        case ofTouchEventArgs::cancel:
            typeString = "CANCEL";
            break;
        default:
            typeString = "UNKNOWN TOUCH TYPE";
            break;
    }

//    ss << "deviceId=" << touch.deviceId << std::endl;

    ss << "type=" << typeString << " (" << touch.type << ")" << std::endl;
    ss << "id=" << touch.id << " @ " << touch.time << std::endl;
//    ss << "\ttapCount=" << 0 << endl;
    ss << "\t#touches=" << touch.numTouches << " angle=" << touch.angle << " pressure=" << touch.pressure << std::endl;
    ss << "\t p =[" << touch.x << "," << touch.y << "]" << std::endl;
    ss << "\t s =[" << touch.width << "," << touch.height << "]" << std::endl;
    ss << "\t ax=[" << touch.minoraxis << "," << touch.majoraxis << "]" << std::endl;
    ss << "\t s =[" << touch.xspeed << "," << touch.yspeed << "]" << std::endl;
    ss << "\t a =[" << touch.xaccel << "," << touch.yaccel << "]" << std::endl;
    
    ss << "-----------------" << std::endl;
    
    return ss.str();
 
}

//------------------------------------------------------------------------------
void ofApp::touchDown(ofTouchEventArgs& touch)
{
    std::cout << touchToString(touch) << std::endl;
}

//------------------------------------------------------------------------------
void ofApp::touchMoved(ofTouchEventArgs& touch)
{
    std::cout << touchToString(touch) << std::endl;
}

//------------------------------------------------------------------------------
void ofApp::touchUp(ofTouchEventArgs& touch)
{
    std::cout << touchToString(touch) << std::endl;
}

//------------------------------------------------------------------------------
void ofApp::touchDoubleTap(ofTouchEventArgs& touch)
{
    std::cout << touchToString(touch) << std::endl;
}

//------------------------------------------------------------------------------
void ofApp::touchCancelled(ofTouchEventArgs& touch)
{
    std::cout << touchToString(touch) << std::endl;
}

//------------------------------------------------------------------------------
void ofApp::keyPressed(int key){
    if (key=='f')
    {
        ofToggleFullscreen();
        ofBackground(0,0,0);
    }
}
