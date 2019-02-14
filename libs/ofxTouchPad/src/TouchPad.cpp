//
// Copyright (c) 2010 Christopher Baker <https://christopherbaker.net>
//
// SPDX-License-Identifier:	GPL
//


#include "ofx/TouchPad.h"
#include "ofMath.h" 
#include "ofLog.h"


namespace ofx {


float TouchPad::_minNormalizedPositionX = 0;
float TouchPad::_minNormalizedPositionY = 0;
float TouchPad::_maxNormalizedPositionX = 1;
float TouchPad::_maxNormalizedPositionY = 1;


void TouchPad::refreshDeviceList()
{
    _deviceList = MTDeviceCreateList();
    
    if (_deviceList == nullptr)
    {
        _nDevices = 0;
        ofLogError("TouchPad::refreshDeviceList") << "MTDeviceCreateList returned NULL.";
    }
    else
    {
        _nDevices = CFArrayGetCount(_deviceList);
    }

    ofLogVerbose("TouchPad::refreshDeviceList") << "MTDeviceCreateList returned " << _nDevices << " devices.";
}


void TouchPad::mt_callback(MTDeviceRef deviceId,
                           MTTouch* touches,
                           int32_t numTouches,
                           double timestamp,
                           int32_t frameNum)
{
    TouchPad& pad = TouchPad::instance();
    
    TouchPad::Touches touchEvents;

    glm::vec2 windowSize(ofGetWidth(), ofGetHeight());
    const auto& scalingRect = pad.getScalingRect();

    for (std::size_t i = 0; i < numTouches; ++i)
    {
        MTTouch* evt = &touches[i];

        ofTouchEventArgs touchEvt;

        touchEvt.id             = evt->pathIndex;
        touchEvt.numTouches     = numTouches;

        // This is a hack to fix non-normalized position data from the multi-touch framework.
        _minNormalizedPositionX = std::min(evt->normalizedVector.position.x, _minNormalizedPositionX);
        _minNormalizedPositionY = std::min(evt->normalizedVector.position.y, _minNormalizedPositionY);
        _maxNormalizedPositionX = std::max(evt->normalizedVector.position.x, _maxNormalizedPositionX);
        _maxNormalizedPositionY = std::max(evt->normalizedVector.position.y, _maxNormalizedPositionY);

        glm::vec2 normPos;
        glm::vec2 normVel;

        normPos.x = ofMap(evt->normalizedVector.position.x,
                          _minNormalizedPositionX,
                          _maxNormalizedPositionX,
                          0,
                          1,
                          true);

        normPos.y = 1.0f - ofMap(evt->normalizedVector.position.y,
                                 _minNormalizedPositionY,
                                 _maxNormalizedPositionY,
                                 0,
                                 1,
                                 true);

        normVel.x = evt->normalizedVector.velocity.x;
        normVel.y = 1.0f - evt->normalizedVector.velocity.y;

        switch (pad.getScalingMode())
        {
            case SCALE_TO_WINDOW:
            {
                touchEvt.x = normPos.x * windowSize.x;
                touchEvt.y = normPos.y * windowSize.y;
                touchEvt.xspeed = normVel.x * windowSize.x;
                touchEvt.yspeed = normVel.y * windowSize.y;
                break;
            }
            case SCALE_TO_RECT:
            {
                touchEvt.x = normPos.x * scalingRect.width + scalingRect.x;
                touchEvt.y = normPos.y * scalingRect.height + scalingRect.y;
                touchEvt.xspeed = normVel.x * scalingRect.width;
                touchEvt.yspeed = normVel.y * scalingRect.height;
                break;
            }
            case NORMALIZED:
            {
                touchEvt.x = normPos.x;
                touchEvt.y = normPos.y;
                touchEvt.xspeed = normVel.x;
                touchEvt.yspeed = normVel.y;
                break;
            }
            case ABSOLUTE:
            {
                touchEvt.x = evt->absoluteVector.position.x;
                touchEvt.y = evt->absoluteVector.position.y;
                touchEvt.xspeed = evt->absoluteVector.velocity.x;
                touchEvt.yspeed = evt->absoluteVector.velocity.y;
                break;
            }
            default:
                ofLogError("TouchPad::mt_callback") << "Unknown scaling mode = " << pad.getScalingMode() << ".";
        }
                           
        touchEvt.xaccel = 0;
        touchEvt.yaccel = 0;

        touchEvt.minoraxis = evt->minorAxis;
        touchEvt.majoraxis = evt->majorAxis;
        
        touchEvt.angle = glm::two_pi<float>() - evt->angle;
        touchEvt.pressure = evt->zTotal;

        if (evt->phase == MTTouchStateMakeTouch)
        {
            touchEvt.type = ofTouchEventArgs::down;
        }
        else if (evt->phase == MTTouchStateTouching)
        {
            touchEvt.type = ofTouchEventArgs::move;
        }
        else if (evt->phase == MTTouchStateOutOfRange)
        {
            touchEvt.type = ofTouchEventArgs::up;
        }
        else
        {
            // Other states are available, but we skip them.
            continue;
        }

        if (touchEvt.id >= 0)
        {
            touchEvents.push_back(touchEvt);
        }
        else
        {
            ofLogError("TouchPad::mt_callback") << "Callback produced an id < 0.";
        }
        
    }

    if (!touchEvents.empty())
    {
        pad.registerTouchEvents(touchEvents);
    }
}


void TouchPad::registerTouchEvents(const Touches& touchEvents)
{
    std::unique_lock<std::mutex> lock(_mutex);
    
    _activeTouches.clear();
    
    auto now = ofGetElapsedTimeMillis();

    for (std::size_t i = 0; i < touchEvents.size(); ++i)
    {
        ofTouchEventArgs t = touchEvents[i];
        
        if (t.type == ofTouchEventArgs::down)
        {
            if ((now - _tapCounts[t.id].lastTap) < _doubleTapSpeed)
            {
                _tapCounts[t.id].tapCount++;
            }
            else
            {
                _tapCounts[t.id].tapCount = 1;
            }
            
            _tapCounts[t.id].lastTap = now; // register last tap time
            
            if (_tapCounts[t.id].tapCount == 2)
            {
                t.type = ofTouchEventArgs::doubleTap;
                ofNotifyEvent(ofEvents().touchDoubleTap, t);
            }

            t.type = ofTouchEventArgs::down;
            ofNotifyEvent(ofEvents().touchDown, t);
            _activeTouches[touchEvents[i].id] = touchEvents[i];
        }
        else if (t.type == ofTouchEventArgs::move)
        {
            ofNotifyEvent(ofEvents().touchMoved, t);
            _activeTouches[touchEvents[i].id] = touchEvents[i];
        }
        else if (t.type == ofTouchEventArgs::up)
        {
            ofNotifyEvent(ofEvents().touchUp, t);
        }
        else
        {
            //
        }
    }
}



TouchPad::TouchPad():
    _scalingMode(SCALE_TO_WINDOW),
    _scalingRectangle(ofRectangle(0,0,ofGetWidth(),ofGetHeight())),
    _doubleTapSpeed(DEFAULT_DOUBLE_TAP_SPEED),
    _exitListener(ofEvents().exit.newListener(this, &TouchPad::exit))
{
    for (std::size_t i = 0; i < MAX_TOUCHES; ++i)
    {
        _tapCounts[i] = TapCount();
    }

    refreshDeviceList();
    connect(); // connect to default device
}


bool TouchPad::connect(int deviceId)
{
    if (0 != _deviceList && deviceId >= 0 && deviceId < _nDevices)
    {
        if (_devices.find(deviceId) == _devices.end())
        {
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

            if (!err)
			{
                rect.set(0, 0, width / 100.0f, height / 100.0f);
            }
			else
			{
                ofLogError("TouchPad::connect") << "Unable to get device dimensions.";
            }
            
            _devices[deviceId] = new DeviceInfo(mtDeviceRef, deviceId, rect);
            
            printDeviceInfo(mtDeviceRef);

            return true;
        }
        else
        {
            ofLogWarning("TouchPad::connect") << "Already connected to device " << deviceId << ".";
            return false;
        }
    }
    else
    {
        ofLogWarning("TouchPad::connect") << "No multitouch devices available.";
        return false;
    }
}


bool TouchPad::disconnect(int deviceId)
{
    if (_deviceList != nullptr && deviceId >= 0 && deviceId < _nDevices)
    {
        auto iter = _devices.find(deviceId);

        if (iter != _devices.end())
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
            ofLogWarning("TouchPad::disconnect") << "Not connected to device " << deviceId << ".";
            return false;
        }
    }
    else
    {
        ofLogWarning("TouchPad::disconnect") << "No multitouch devices available.";
        return false;
    }
}


std::size_t TouchPad::getNumDevices() const
{
    return _nDevices;
}


TouchPad::~TouchPad()
{
}


void TouchPad::exit(ofEventArgs& etc)
{
    // Ensure that it is disabled on destruction.
    if (_disableOSGestureSupport)
    {
        enableOSGestureSupport();
    }

    auto iter = _devices.begin();

    while (iter != _devices.end())
    {
        if (!disconnect(iter->first))
        {
            ofLogError("TouchPad::~TouchPad") << "Unable to disconnect from " << iter->first;
        }
        else
        {
            // Successfully disconnected;
            return;
        }

        ++iter;
    }

    ofLogVerbose("TouchPad::~TouchPad") << "Multitouch devices have been disconnected.";
}


std::size_t TouchPad::touchCount() const
{
    std::unique_lock<std::mutex> lock(_mutex);
    return _activeTouches.size();
}


std::size_t TouchPad::getTouchCount() const
{
    return touchCount();
}


TouchPad::Touches TouchPad::touches() const
{
    std::unique_lock<std::mutex> lock(_mutex);

    Touches touches;

    for (const auto& touch: _activeTouches)
    {
        touches.push_back(touch.second);
    }

    return touches;
}


TouchPad::Touches TouchPad::getTouches() const
{
    return touches();
}


TouchPad::TouchMap TouchPad::touchMap() const
{
    std::unique_lock<std::mutex> lock(_mutex);
    return _activeTouches;
}


TouchPad::TouchMap TouchPad::getTouchMap() const
{
    return touchMap();
}


bool TouchPad::hasTouchId(int touchId) const
{
    std::unique_lock<std::mutex> lock(_mutex);
    return _activeTouches.find(touchId) != _activeTouches.end();
}


uint64_t TouchPad::getDoubleTapSpeed() const
{
    return _doubleTapSpeed;
}


void TouchPad::setDoubleTapSpeed(uint64_t doubleTapSpeed)
{
    std::unique_lock<std::mutex> lock(_mutex);
    _doubleTapSpeed = doubleTapSpeed;
}


TouchPad::ScalingMode TouchPad::getScalingMode() const
{
    std::unique_lock<std::mutex> lock(_mutex);
    return _scalingMode;
}


void TouchPad::setScalingMode(ScalingMode scalingMode)
{
    std::unique_lock<std::mutex> lock(_mutex);
    _scalingMode = scalingMode;
}


const ofRectangle& TouchPad::getScalingRect() const
{
    std::unique_lock<std::mutex> lock(_mutex);
    return _scalingRectangle;
}


void TouchPad::setScalingRect(const ofRectangle& scalingRectangle)
{
    std::unique_lock<std::mutex> lock(_mutex);
    _scalingRectangle = scalingRectangle;
}


void TouchPad::disableCoreMouseEvents()
{
    ofEvents().mouseMoved.disable();
    ofEvents().mouseDragged.disable();
    ofEvents().mousePressed.disable();
    ofEvents().mouseReleased.disable();

    ofEvents().mouseScrolled.disable();
    ofEvents().mouseEntered.disable();
    ofEvents().mouseExited.disable();
}


void TouchPad::enableCoreMouseEvents()
{
    ofEvents().mouseMoved.enable();
    ofEvents().mouseDragged.enable();
    ofEvents().mousePressed.enable();
    ofEvents().mouseReleased.enable();

    ofEvents().mouseScrolled.enable();
    ofEvents().mouseEntered.enable();
    ofEvents().mouseExited.enable();
}


void TouchPad::enableOSGestureSupport()
{
    _disableOSGestureSupport = false;

#if defined(TARGET_OSX)
    //  The following code re-enables default system-wide gesture support.
    ofSystem("killall -CONT Dock"); // turn on OS level gesture support
    CGAssociateMouseAndMouseCursorPosition(true);
    ofShowCursor();
#endif

}


void TouchPad::disableOSGestureSupport()
{
    _disableOSGestureSupport = true;

#if defined(TARGET_OSX)
    // The following code attempts to prevent conflicts between system-wide
    // gesture support and the raw TouchPad data provided by ofxTouchPad.
    ofSystem("killall -STOP Dock"); // turn off OS level gesture support ...
    CGAssociateMouseAndMouseCursorPosition(false);
    ofHideCursor();
#endif

}


std::string TouchPad::touchPhaseToString(MTTouchPhase phase)
{
    switch (phase)
    {
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

    return "UNKNOWN";
}


void TouchPad::printDeviceInfo(MTDeviceRef deviceRef)
{
    uuid_t guid;

    if (!MTDeviceGetGUID(deviceRef, &guid))
    {
        uuid_string_t val;
        uuid_unparse(guid, val);
        ofLogNotice("TouchPad::printDeviceInfo") << val;
    }

    bool supportsActuation = MTDeviceSupportsActuation(deviceRef);
    bool supportsForce = MTDeviceSupportsForce(deviceRef);

    ofLogVerbose("TouchPad::printDeviceInfo") << "Supports Actuation: " << supportsActuation;
    ofLogVerbose("TouchPad::printDeviceInfo") << "Supports Force: " << supportsForce;

    int a;

    if (!MTDeviceGetDriverType(deviceRef, &a))
    {
        ofLogVerbose("TouchPad::printDeviceInfo") << "Driver type: " << a;

        int _a;
//        if (!MTDeviceGetActualType(deviceRef, &_a))
//        {
//            ofLogVerbose("TouchPad::printDeviceInfo") << "Actual Driver type: " << _a;
//        }
    }
    
    if (MTDeviceGetDeviceID)
    {
        uint64_t devID;
        
        if (!MTDeviceGetDeviceID(deviceRef, &devID))
        {
            ofLogVerbose("TouchPad::printDeviceInfo") << "Device ID: " << devID;
        }
    }

    if (!MTDeviceGetFamilyID(deviceRef, &a))
    {
        ofLogVerbose("TouchPad::printDeviceInfo") << "Family ID: " << a;
    }
    
    int b;

    if (!MTDeviceGetSensorSurfaceDimensions(deviceRef, &a, &b))
    {
        ofLogVerbose("TouchPad::printDeviceInfo") << "Dimensions: " << a / 100.0f << " x " << b / 100.0;
    }

    if (!MTDeviceGetSensorDimensions(deviceRef, &a, &b))
    {
        ofLogVerbose("TouchPad::printDeviceInfo") << "Rows: " << a << " Columns: " << b;
    }

    if (MTDeviceIsBuiltIn)
    {
        if (MTDeviceIsBuiltIn(deviceRef))
        {
            ofLogVerbose("TouchPad::printDeviceInfo") << "Is device built-in: YES";
        }
        else
        {
            ofLogVerbose("TouchPad::printDeviceInfo") << "Is device built-in: NO";
        }
    }
}


TouchPad& TouchPad::instance()
{
    static TouchPad sh;
    return sh;
}


} // namespace ofx
