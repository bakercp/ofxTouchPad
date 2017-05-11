//
// Copyright (c) 2010 Christopher Baker <https://christopherbaker.net>
//
// SPDX-License-Identifier:	GPL
//


#include "ofApp.h"


void ofApp::setup()
{
    ofx::RegisterPointerEvents(this);

    ofSetLogLevel(OF_LOG_VERBOSE);
    ofSetFrameRate(120);
    ofEnableAlphaBlending();
    ofEnableSmoothing();
    
    TouchPad& pad = TouchPad::instance();

    pad.setScalingRect(ofRectangle(100, 100, 160 * 5, 120 * 5));
    pad.setScalingMode(TouchPad::SCALE_TO_RECT);
    
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
    
    TouchPad& pad = TouchPad::instance();

    ofSetColor(255,255,255);
    ofDrawBitmapString("TouchCount: " + ofToString(pad.getTouchCount(), 0), 20, 20);
    
    // Simple
    ofFill();
    ofSetColor(255, 255, 255, 100);

    ofRectangle scalingRect = pad.getScalingRect();
    
    ofDrawRectRounded(scalingRect, 10);
    
    TouchPad::Touches touches = pad.getTouches();

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
        ofDrawBitmapString(ofToString(touches[i].id), touches[i].x-6, touches[i].y+3);
        ofSetColor(255, 255, 255, 100);
    }
}


void ofApp::onPointerDown(PointerEventArgs& evt)
{
    ofLogNotice("ofApp::onPointerDown") << evt.toString();
}


void ofApp::onPointerUp(PointerEventArgs& evt)
{
    ofLogVerbose("ofApp::onPointerUp") << evt.toString();
}


void ofApp::onPointerMove(PointerEventArgs& evt)
{
    ofLogVerbose("ofApp::onPointerMove") << evt.toString();
}


void ofApp::onPointerCancel(PointerEventArgs& evt)
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
