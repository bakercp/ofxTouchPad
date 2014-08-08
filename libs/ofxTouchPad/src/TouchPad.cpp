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


#include "ofx/TouchPad.h"


namespace ofx {


void TouchPad::refreshDeviceList()
{
    _deviceList = MTDeviceCreateList();
    
    if(_deviceList == NULL)
    {
        _nDevices = 0;
        ofLogError("TouchPad") << "MTDeviceCreateList returned NULL.";
    }
    else
    {
        _nDevices = CFArrayGetCount(_deviceList);
    }

    ofLogVerbose("TouchPad") << "MTDeviceCreateList returned " << _nDevices << " devices.";
}


void TouchPad::mt_callback(MTDeviceRef deviceId,
                           MTTouch* touches,
                           int32_t numTouches,
                           double timestamp,
                           int32_t frameNum)
{
    TouchPad& pad = TouchPad::getTouchPadRef();
    
    TouchPad::Touches touchEvents;
    
    for(std::size_t i = 0; i < numTouches; ++i)
    {
        MTTouch* evt = &touches[i];
        
        ofTouchEventArgs touchEvt;

        touchEvt.id             = evt->pathIndex;
        touchEvt.numTouches     = numTouches;
        
// touchEvt.timestamp    = ofGetElapsedTimeMillis();
// touchEvt.frameNumber  = ofGetFrameNum();
        
        switch (pad.getScalingMode())
        {
            case SCALE_TO_WINDOW:
            {
                touchEvt.x  =         evt->normalizedVector.position.x  * ofGetWidth();
                touchEvt.y  = (1.0f - evt->normalizedVector.position.y) * ofGetHeight();
//                touchEvt.velocity.x =         evt->normalizedVector.velocity.x  * ofGetWidth();
//                touchEvt.velocity.y = (1.0f - evt->normalizedVector.velocity.y) * ofGetHeight();
                touchEvt.xspeed =         evt->normalizedVector.velocity.x  * ofGetWidth();
                touchEvt.yspeed = (1.0f - evt->normalizedVector.velocity.y) * ofGetHeight();
                break;
            }
            case SCALE_TO_RECT:
            {
                ofRectangle r = pad.getScalingRect();
                touchEvt.x      = evt->normalizedVector.position.x * r.width  + r.x;
                touchEvt.y      = (1.0f - evt->normalizedVector.position.y) * r.height + r.y;
//                touchEvt.velocity.x = evt->normalizedVector.velocity.x * r.width  + r.x;
//                touchEvt.velocity.y = (1.0f - evt->normalizedVector.velocity.y) * r.height + r.y;
                touchEvt.xspeed = evt->normalizedVector.velocity.x * r.width  + r.x;
                touchEvt.yspeed = (1.0f - evt->normalizedVector.velocity.y) * r.height + r.y;
                break;
            }
            case NORMALIZED:
            {
                touchEvt.x      = (1.0f - evt->normalizedVector.position.x);
                touchEvt.y      =         evt->normalizedVector.position.y;
//                touchEvt.velocity.x =         evt->normalizedVector.velocity.x;
//                touchEvt.velocity.y = (1.0f - evt->normalizedVector.velocity.y);
                touchEvt.xspeed =         evt->normalizedVector.velocity.x;
                touchEvt.yspeed = (1.0f - evt->normalizedVector.velocity.y);
                break;
            }
            case ABSOLUTE:
            {
                touchEvt.x      = evt->absoluteVector.position.x;
                touchEvt.y      = evt->absoluteVector.position.y;
//                touchEvt.velocity.x = evt->absoluteVector.velocity.x;
//                touchEvt.velocity.y = evt->absoluteVector.velocity.y;
                touchEvt.xspeed = evt->absoluteVector.velocity.x;
                touchEvt.yspeed = evt->absoluteVector.velocity.y;
                break;
            }
            default:
                ofLogError("TouchPad") << "Unknown scaling mode = " << pad.getScalingMode() << ".";
        }
                           
//        touchEvt.acceleration.set(0,0,0);
        touchEvt.xaccel = 0;
        touchEvt.yaccel = 0;

        touchEvt.minoraxis = evt->minorAxis;
        touchEvt.majoraxis = evt->majorAxis;
        
//        touchEvt.contactSize = (ofVec2f(evt->minorAxis,evt->majorAxis));
        touchEvt.angle          = TWO_PI - evt->angle;
        touchEvt.pressure       = evt->zTotal;
//        touchEvt.deviceId       = (unsigned int)deviceId;
        
        if(evt->phase == MTTouchStateMakeTouch)
        {
            touchEvt.type = ofTouchEventArgs::down;
        }
        else if(evt->phase == MTTouchStateTouching)
        {
            touchEvt.type = ofTouchEventArgs::move;
        }
        else if(evt->phase == MTTouchStateOutOfRange)
        {
            touchEvt.type = ofTouchEventArgs::up;
        }
        else
        {
            // other states are available, but we skip t them
            continue;
        }

        if(touchEvt.id >= 0)
        {
            touchEvents.push_back(touchEvt);
        }
        else
        {
            ofLogError("TouchPad") << "Callback produced an id < 0.";
        }
        
    }

    if(!touchEvents.empty())
    {
        pad.registerTouchEvents(touchEvents);
    }

}


void TouchPad::registerTouchEvents(const Touches& touchEvents)
{
    ofScopedLock lock(_mutex);
    
    _activeTouches.clear();
    
    unsigned long long now = ofGetElapsedTimeMillis();

    for(std::size_t i = 0; i < touchEvents.size(); ++i)
    {
        ofTouchEventArgs t = touchEvents[i];
        
        if(t.type == ofTouchEventArgs::down)
        {
            if((now - _tapCounts[t.id].lastTap) < _doubleTapSpeed)
            {
                _tapCounts[t.id].tapCount++;
            }
            else
            {
                _tapCounts[t.id].tapCount = 1;
            }
            
            _tapCounts[t.id].lastTap = now; // register last tap time
            
//            t.tapCount = _tapCounts[t.id].tapCount; // record tap count
//            // send double tap first like ios
            if(_tapCounts[t.id].tapCount == 2) {
                t.type = ofTouchEventArgs::doubleTap;
                ofNotifyEvent(ofEvents().touchDoubleTap, t);
            }

            t.type = ofTouchEventArgs::down;
//            ofNotifyEvent(ofWindowEvents().touchDown, t);
            ofNotifyEvent(ofEvents().touchDown, t);
            _activeTouches[touchEvents[i].id] = touchEvents[i];
        }
        else if(t.type == ofTouchEventArgs::move)
        {
//            ofNotifyEvent(ofWindowEvents().touchMoved, t);
            ofNotifyEvent(ofEvents().touchMoved, t);
            _activeTouches[touchEvents[i].id] = touchEvents[i];
        }
        else if(t.type == ofTouchEventArgs::up)
        {
//            ofNotifyEvent(ofWindowEvents().touchUp, t);
            ofNotifyEvent(ofEvents().touchUp, t);
        }
        else
        {
            //
        }
    }
}



TouchPad::TouchPad():
    _doubleTapSpeed(DEFAULT_DOUBLE_TAP_SPEED),
    _scalingMode(SCALE_TO_WINDOW),
    _scalingRectangle(ofRectangle(0,0,ofGetWidth(),ofGetHeight()))
{
    for(std::size_t i = 0; i < MAX_TOUCHES; ++i)
    {
        _tapCounts[i] = TapCount();
    }

    refreshDeviceList();
    connect(); // connect to default device
}


bool TouchPad::connect(int deviceId)
{

    if(_deviceList != NULL && deviceId >= 0 && deviceId < _nDevices)
    {
        if (_devices.find(deviceId) == _devices.end()) {
            // get the device reference
            MTDeviceRef mtDeviceRef = (MTDeviceRef)CFArrayGetValueAtIndex(_deviceList, deviceId);
            // register the callback for the reference
            MTRegisterContactFrameCallback(mtDeviceRef, mt_callback);
            // start the device
            MTDeviceStart(mtDeviceRef);
            // store a reference w/ a device number

            int32_t width  = -1;
            int32_t height = -1;
            
            OSStatus err = MTDeviceGetSensorSurfaceDimensions(mtDeviceRef, &width, &height);

            ofRectangle rect;

            if(!err) {
                rect.set(0,0,width/100.0f,height/100.0f);
            } else {
                ofLogError("TouchPad") << "Unable to get device dimensions.";
            }
            
            _devices[deviceId] = new DeviceInfo(mtDeviceRef,deviceId,rect);
            
            printDeviceInfo(mtDeviceRef);

            return true;
        } else {
            ofLogWarning("TouchPad") << "Already connected to device " << deviceId << ".";
            return false;
        }
    } else {
        ofLogWarning("TouchPad") << "No multitouch devices available.";
        return false;
    }
}


bool TouchPad::disconnect(int deviceId)
{
    if(_deviceList != NULL && deviceId >= 0 && deviceId < _nDevices)
    {
        std::map<int,DeviceInfo*>::iterator iter = _devices.find(deviceId);

        if(iter != _devices.end())
        {
            MTDeviceStop(iter->second->ref);
            MTUnregisterContactFrameCallback(iter->second->ref, mt_callback);
            MTDeviceRelease(iter->second->ref);
            delete iter->second; // deallocate
            _devices.erase(iter); // remove it from the list
            return true;
        }
        else
        {
            ofLogWarning("TouchPad") << "Not connected to device " << deviceId << ".";
            return false;
        }
    }
    else
    {
        ofLogWarning("TouchPad") << "No multitouch devices available.";
        return false;
    }
}


std::size_t TouchPad::getNumDevices() const
{
    return _nDevices;
}


TouchPad::~TouchPad()
{
    DeviceMap::const_iterator iter = _devices.begin();
    
    while(iter != _devices.end())
    {
        if(!disconnect(iter->first))
        {
            ofLogError("TouchPad") << "Unable to disconnect from " << iter->first;
        }
        ++iter;
    }

    ofLogVerbose("TouchPad") << "Multitouch devices have been disconnected.";
}


std::size_t TouchPad::getTouchCount() const
{
    return _activeTouches.size();
}


TouchPad::Touches TouchPad::getTouches() const
{
    ofScopedLock lock(_mutex);

    Touches touches;

    TouchMap::const_iterator iter = _activeTouches.begin();

    while(iter != _activeTouches.end())
    {
        touches.push_back(iter->second);
        ++iter;
    }

    return touches;
    
}


TouchPad::TouchMap TouchPad::getTouchMap() const
{
    return _activeTouches;
}


bool TouchPad::hasTouchId(int touchId) const
{
    return _activeTouches.find(touchId) != _activeTouches.end();
}


unsigned long long TouchPad::getDoubleTapSpeed() const
{
    return _doubleTapSpeed;
}


void TouchPad::setDoubleTapSpeed(unsigned long long doubleTapSpeed)
{
    _doubleTapSpeed = doubleTapSpeed;
}


TouchPad::ScalingMode TouchPad::getScalingMode() const
{
    return _scalingMode;
}


void TouchPad::setScalingMode(ScalingMode scalingMode)
{
    _scalingMode = scalingMode;
}


ofRectangle TouchPad::getScalingRect() const
{
    return _scalingRectangle;
}


void TouchPad::setScalingRect(const ofRectangle& scalingRectangle)
{
    _scalingRectangle = scalingRectangle;
}


std::string TouchPad::touchPhaseToString(MTTouchPhase phase)
{
    switch(phase) {
        case MTTouchStateNotTracking:
            return "MTTouchStateNotTracking";
        case MTTouchStateStartInRange:
            return "MTTouchStateStartInRange";
        case MTTouchStateHoverInRange:
            return "MTTouchStateHoverInRange";
        case MTTouchStateMakeTouch:
            return "MTTouchStateMakeTouch";
        case MTTouchStateTouching:
            return "MTTouchStateTouching";
        case MTTouchStateBreakTouch:
            return "MTTouchStateBreakTouch";
        case MTTouchStateLingerInRange:
            return "MTTouchStateLingerInRange";
        case MTTouchStateOutOfRange:
            return "MTTouchStateOutOfRange";
        default:
            return "MTTouchPhaseInvalid";
    }
}


void TouchPad::printDeviceInfo(MTDeviceRef deviceRef)
{
    uuid_t guid;

    if(!MTDeviceGetGUID(deviceRef, &guid))
    {
        uuid_string_t val;
        uuid_unparse(guid, val);
        printf("%s ", val);
    }
    
    int a;

    if (!MTDeviceGetDriverType(deviceRef, &a))
    {
        ofLogVerbose("TouchPad") << "Driver type: " << a;
//        if(!MTDeviceGetActualType(deviceRef, &a))
//        {
//            ofLogVerbose("TouchPad") << "Actual Driver type: " << a;
//        }
    }
    
    if (MTDeviceGetDeviceID)
    {
        uint64_t devID;
        
        if (!MTDeviceGetDeviceID(deviceRef, &devID))
        {
            ofLogVerbose("TouchPad") << "Device ID: " << devID;
        }
    }

    if(!MTDeviceGetFamilyID(deviceRef, &a))
    {
        ofLogVerbose("TouchPad") << "Family ID: " << a;
    }
    
    int b;

    if(!MTDeviceGetSensorSurfaceDimensions(deviceRef, &a, &b))
    {
        ofLogVerbose("TouchPad") << "Dimensions: " << a/100.0 << " x " << b/100.0;
    }

    if(!MTDeviceGetSensorDimensions(deviceRef, &a, &b))
    {
        ofLogVerbose("TouchPad") << "Rows: " << a << " Columns: " << b;
    }

    if (MTDeviceIsBuiltIn)
    {
        if(MTDeviceIsBuiltIn(deviceRef))
        {
            ofLogVerbose("TouchPad") << "Is device built-in: YES";
        }
        else
        {
            ofLogVerbose("TouchPad") << "Is device built-in: NO";
        }
    }
}


TouchPad& TouchPad::getTouchPadRef()
{
    static Poco::SingletonHolder<TouchPad> sh;
    return *sh.get();
}


} // ofx
