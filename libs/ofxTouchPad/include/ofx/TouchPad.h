//
// Copyright (c) 2010 Christopher Baker <https://christopherbaker.net>
//
// SPDX-License-Identifier:	GPL
//

// Based on code from https://github.com/calftrail/TrackMagic (GPL v2.0)
// Based on code from Jens Alexander Ewald http://www.520at.net
// Based on code from Steike http://steike.com/code/multitouch

////////////////////////////////////////////////////////////////////////////////
// IMPORTANT!
// You must include the MultitouchSupport.framework from:
// /System/Library/PrivateFrameworks/MultitouchSupport.framework
// in order to compile.
////////////////////////////////////////////////////////////////////////////////


#pragma once


#ifndef __APPLE_CC__
    #error -- Currently this code only works with MAC OS X >= 10.5
#else
    #if (MAC_OS_X_VERSION_MAX_ALLOWED < MAC_OS_X_VERSION_10_5)
        #error -- This needs OS X 10.5 (Leopard) or higher!\n \
            \t\tCheck your current SDK settings and add the \
            \t\/tMultitouchSupport.framework from the PrivateFrameworks folder
    #endif
#endif


#include <cstdint>
#include <map>
#include "ofAppRunner.h"
#include "ofEvents.h"
#include "ofRectangle.h"
#include "ofUtils.h"
#include "MTTypes.h"
#include "Poco/SingletonHolder.h"


namespace ofx {


class TapCount
{
public:
    TapCount() : lastTap(0), tapCount(0)
    {
    }

    ~TapCount()
    {
    }

    uint64_t lastTap;
    uint64_t tapCount;
};

class DeviceInfo
{
public:
    DeviceInfo(MTDeviceRef _ref, int _id, const ofRectangle& _rect):
        ref(_ref),
        id(_id),
        rect(_rect)
    {
    }
    
    ~DeviceInfo()
    {
    }
    
    MTDeviceRef ref;
    int id;
    ofRectangle rect;
};


class TouchPad
{
public:
    typedef std::map<int, ofTouchEventArgs> TouchMap;
    typedef std::vector<ofTouchEventArgs>  Touches;

    enum ScalingMode
    {
        SCALE_TO_WINDOW = 0,
        SCALE_TO_RECT   = 1,
        NORMALIZED      = 2,
        ABSOLUTE        = 3
    };

    std::size_t getNumDevices() const;

    bool connect(int deviceId = DEFAULT_DEVICE_ID);
    bool disconnect(int deviceId = DEFAULT_DEVICE_ID);

    std::size_t getTouchCount() const;

    Touches  getTouches() const;
    TouchMap getTouchMap() const;

    bool hasTouchId(int touchId) const;
    
    uint64_t getDoubleTapSpeed() const;
    void setDoubleTapSpeed(uint64_t doubleTapSpeed);
    
    ScalingMode getScalingMode() const;
    void setScalingMode(ScalingMode scalingMode);

    ofRectangle getScalingRect() const;
    void setScalingRect(const ofRectangle& scalingRect);

    void disableCoreMouseEvents();
    void enableCoreMouseEvents();

    static TouchPad& instance();

    enum
    {
        DEFAULT_DEVICE_ID = 0,
        DEFAULT_DOUBLE_TAP_SPEED = 500
    };

private:
    typedef std::map<int, DeviceInfo*> DeviceMap;

    enum
    {
        MAX_TOUCHES = 1024

    };

    // singleton
    TouchPad();
    virtual ~TouchPad();
    TouchPad(const TouchPad&);
    TouchPad& operator=(const TouchPad&);
    
    void registerTouchEvents(const Touches& touchEvents);
    
    static void mt_callback(MTDeviceRef deviceId,
                            MTTouch* touches,
                            int32_t numTouches,
                            double timestamp,
                            int32_t frameNum);

    
    ScalingMode _scalingMode;
    ofRectangle _scalingRectangle;
    
    void refreshDeviceList();
    CFMutableArrayRef _deviceList;
    std::size_t _nDevices;
    
    TouchMap _activeTouches;
    DeviceMap _devices;
    
    uint64_t _doubleTapSpeed; // ms
    TapCount _tapCounts[MAX_TOUCHES];   // an internal tap counter

    static std::string touchPhaseToString(MTTouchPhase phase);
    static void printDeviceInfo(MTDeviceRef d);
    
    mutable std::mutex _mutex; // to synchronize the system and oF event threads

    friend class Poco::SingletonHolder<TouchPad>;
};
    

} // ofx
