//
// Copyright (c) 2010 Christopher Baker <https://christopherbaker.net>
//
// SPDX-License-Identifier:	GPL
//


#pragma once


#include "ofMain.h"
#include "ofxTouchPad.h"


class ofApp: public ofBaseApp
{
public:
    void setup() override;
    void draw() override;
    void exit() override;

    void keyPressed(int key) override;

    void touchDown(ofTouchEventArgs& touch) override;
    void touchMoved(ofTouchEventArgs& touch) override;
    void touchUp(ofTouchEventArgs& touch) override;
    void touchDoubleTap(ofTouchEventArgs& touch) override;
    void touchCancelled(ofTouchEventArgs& touch) override;

    std::string to_string(const ofTouchEventArgs& touch);

};
