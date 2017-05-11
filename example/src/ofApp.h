//
// Copyright (c) 2010 Christopher Baker <https://christopherbaker.net>
//
// SPDX-License-Identifier:	GPL
//


#pragma once


#include "ofMain.h"
#include "ofxTouchPad.h"


using namespace ofx;


class ofApp: public ofBaseApp
{
public:
    void setup();
    void draw();
    void exit();

    void keyPressed(int key);

    void onPointerDown(PointerEventArgs& evt);
    void onPointerUp(PointerEventArgs& evt);
    void onPointerMove(PointerEventArgs& evt);
    void onPointerCancel(PointerEventArgs& evt);

};
