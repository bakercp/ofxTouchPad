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
    
    ofx::TouchPad& pad = ofx::TouchPad::instance();

    pad.setScalingRect(ofRectangle(100, 100, 160 * 5, 120 * 5));
    pad.setScalingMode(ofx::TouchPad::SCALE_TO_RECT);

    // The following code attempts to prevent conflicts between system-wide
    // gesture support and the raw TouchPad data provided by ofxTouchPad.
    //
    // It also means that the dock will be disabled, which can be annoying in
    // multi-tasking situations.
    ofx::TouchPad::instance().disableOSGestureSupport();

}


void ofApp::exit()
{
    // The following code re-enables default system-wide gesture support.
    ofx::TouchPad::instance().enableOSGestureSupport();
}


void ofApp::draw()
{
    ofBackground(0);
    
    auto& pad = ofx::TouchPad::instance();

    ofDrawBitmapString("TouchCount: " + ofToString(pad.touchCount(), 0), 20, 20);
    
    ofFill();
    ofSetColor(255, 255, 255, 100);
    ofDrawRectRounded(pad.getScalingRect(), 10);
    
    for (const auto& touch: pad.touches())
    {
        ofPushMatrix();
        ofTranslate(touch);
        
        float w = touch.majoraxis * 5;
        float h = touch.minoraxis * 5;
        float halfW = w / 2.0f;
        float halfH = h / 2.0f;

        float pressure = touch.pressure * 20;
        
        ofRotateZRad(touch.angle);
        ofSetColor(255, 100);
        ofDrawEllipse(0, 0, w, h);
        ofSetColor(255, 255,0, 100);
        ofDrawEllipse(0,0, pressure, pressure);
        ofSetColor(255,100);
        ofDrawLine(-halfW, 0.0f, halfW, 0.0f);
        ofDrawLine(0.0f, -halfH, 0.0f, halfH);
        ofPopMatrix();
        
        ofFill();
        ofSetColor(255);
        ofDrawBitmapString(ofToString(touch.id), touch.x - 6, touch.y + 3);
        ofSetColor(255, 255, 255, 100);
    }
}


void ofApp::keyPressed(int key)
{
    if (key == 'f')
    {
        ofToggleFullscreen();
        ofBackground(0,0,0);
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
