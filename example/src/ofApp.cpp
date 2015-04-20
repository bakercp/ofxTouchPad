// =============================================================================
//
// Copyright (c) 2010-2015 Christopher Baker <http://christopherbaker.net>
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


void ofApp::setup()
{
    ofx::RegisterPointerEvents(this);

    ofSetLogLevel(OF_LOG_VERBOSE);
    ofSetFrameRate(120);
    ofEnableAlphaBlending();
    ofEnableSmoothing();
    
    ofx::TouchPad& pad = ofx::TouchPad::getTouchPadRef();

    pad.setScalingRect(ofRectangle(100, 100, 160 * 5, 120 * 5));
    pad.setScalingMode(ofx::TouchPad::SCALE_TO_RECT);
    
    //  The following code attempts to prevent conflicts between system-wide
    //  gesture support and the raw TouchPad data provided by ofxTouchPad.
        ofSystem("killall -STOP Dock"); // turn off OS level gesture support ...
        CGAssociateMouseAndMouseCursorPosition(false);
        ofHideCursor();
}


void ofApp::exit()
{
    //  The following code re-enables default system-wide gesture support.
        ofSystem("killall -CONT Dock"); // turn on OS level gesture support
        CGAssociateMouseAndMouseCursorPosition(true);
        ofShowCursor();
}


void ofApp::draw()
{
    ofBackground(0);
    
    ofx::TouchPad& pad = ofx::TouchPad::getTouchPadRef();

    ofSetColor(255,255,255);
    ofDrawBitmapString("TouchCount: " + ofToString(pad.getTouchCount(), 0), 20, 20);
    
    // Simple
    ofFill();
    ofSetColor(255, 255, 255, 100);

    ofRectangle scalingRect = pad.getScalingRect();
    
    ofDrawRectRounded(scalingRect, 10);
    
    ofx::TouchPad::Touches touches = pad.getTouches();

    for (std::size_t i = 0; i < touches.size(); ++i)
    {
        ofPushMatrix();
        ofTranslate(touches[i].x, touches[i].y);
        
        float w = touches[i].majoraxis * 5;
        float h = touches[i].minoraxis * 5;
        
        float pressure = touches[i].pressure * 20;
        
        ofRotateZ(ofRadToDeg(touches[i].angle));
        ofSetColor(255,100);
        ofDrawEllipse(0,0,w,h);
        ofSetColor(255,255,0,100);
        ofDrawEllipse(0,0,pressure,pressure);
        ofSetColor(255,100);
        ofDrawLine(-w/2.0f,0.0f,w/2.0f,0.0f);
        ofDrawLine(0.0f,-h/2.0f,0.0f,h/2.0f);
        ofPopMatrix();
        
        ofFill();
        ofSetColor(255);
        ofDrawBitmapString(ofToString(touches[i].id),touches[i].x-6, touches[i].y+3);
        ofSetColor(255, 255, 255, 100);
    }
}


void ofApp::onPointerDown(ofx::PointerEventArgs& evt)
{
    ofLogNotice("ofApp::onPointerDown") << evt.toString();
}


void ofApp::onPointerUp(ofx::PointerEventArgs& evt)
{
    ofLogVerbose("ofApp::onPointerUp") << evt.toString();
}


void ofApp::onPointerMove(ofx::PointerEventArgs& evt)
{
    ofLogVerbose("ofApp::onPointerMove") << evt.toString();
}


void ofApp::onPointerCancel(ofx::PointerEventArgs& evt)
{
    ofLogVerbose("ofApp::onPointerCancel") << evt.toString();
}


void ofApp::keyPressed(int key)
{
    if (key == 'f')
    {
        ofToggleFullscreen();
        ofBackground(0,0,0);
    }
}
