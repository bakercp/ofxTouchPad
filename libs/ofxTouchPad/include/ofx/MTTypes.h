/* ------------------------------------------------------------------------- */
/*  MTTypes.h                                                                */
/* ------------------------------------------------------------------------- */
/*                                                                           */
/*  Created by jens a. ewald on 26.10.09.                                    */
/*                                                                           */
/*  http://www.openFrameworks.cc                                             */
/*                                                                           */
/*  access the data from the multitouchtrackpad sensor as addon for OF       */
/*                                                                           */
/*  based on the code at http://steike.com/code/multitouch                   */
/*  & the puredata external by Hans-Christoph Steiner                        */
/*                                                                           */
/*                                                                           */
/*  Copyright (c) 2009 jens alexander ewald http://www.520at.net             */
/*  Copyright (c) 2009 Hans-Christoph Steiner                                */
/*  Copyright (c) 2008 Steike                                                */
/*                                                                           */
/*                                                                           */
/* This program is free software; you can redistribute it and/or             */
/* modify it under the terms of the GNU General Public License               */
/* as published by the Free Software Foundation; either version 3            */
/* of the License, or (at your option) any later version.                    */
/*                                                                           */
/* See file LICENSE for further informations on licensing terms.             */
/*                                                                           */
/* This program is distributed in the hope that it will be useful,           */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             */
/* GNU General Public License for more details.                              */
/*                                                                           */
/* You should have received a copy of the GNU General Public License         */
/* along with this program; if not, write to the Free Software Foundation,   */
/* Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA */
/*                                                                           */
/* ------------------------------------------------------------------------- */

// https://github.com/calftrail/TrackMagic/blob/master/MultitouchSupport.h
// https://github.com/INRIA/libpointing/blob/master/pointing/input/osx/osxPrivateMultitouchSupport.h

/* 

 IMPORTANT!
 You must include the MultitouchSupport.framework from in your project.
 You can find it here:
    /System/Library/PrivateFrameworks/MultitouchSupport.framework

*/

#pragma once

#include <mach/mach.h> 
#include <IOKit/IOKitLib.h> 
#include <CoreFoundation/CoreFoundation.h> 

#ifdef __cplusplus
extern "C" {
#endif
    typedef enum
    {
        VERBOSE = 0,
        LESS_VERBOSE = 0x10000000
    } MTRunMode;

    typedef enum
    {
        OFF = 0,
        ON = 1
    } MTEasyCallbackState;

    typedef enum {
        MTTouchStateNotTracking   = 0,
        MTTouchStateStartInRange  = 1,
        MTTouchStateHoverInRange  = 2,
        MTTouchStateMakeTouch     = 3,
        MTTouchStateTouching      = 4,
        MTTouchStateBreakTouch    = 5,
        MTTouchStateLingerInRange = 6,
        MTTouchStateOutOfRange    = 7
    } MTTouchPhase;
    
    typedef struct {
        float x;
        float y;
    } MTPoint;
    
    typedef struct {
        MTPoint position;
        MTPoint velocity;
    } MTVector;
    
    typedef struct {
        int32_t  frame;            // the current frame
        double   timestamp;        // event timestamp
        int32_t  pathIndex;        // "P" (~transducerIndex)
        MTTouchPhase phase;        // the current state (not sure what the values mean)
        int32_t  fingerID;	       // "F" (~identity)
        int32_t  handID;		   // "H" (always 1)
        MTVector normalizedVector; // normalized position / velocity vector
        float    zTotal;           // the "area" of the touch (larger finger pad / more pressure => bigger size)
                                   // "ZTot" (~quality, multiple of 1/8 between 0 and 1)
        int32_t  UNKNOWN_0;        // unknown
        float    angle;            // angle of touch ellipse
        float    majorAxis;        // major axis of touch ellipse
        float    minorAxis;        // minor axis of touch ellipse
        MTVector absoluteVector;   // "mm"
        int32_t  UNKNOWN_1;        // unknown -- always 0/0?
        int32_t  UNKNOWN_2;        // unknown -- always 0/0?
        float    zDensity;		   // "ZDen" (~density)
    } MTTouch;
    
    typedef CFTypeRef MTDeviceRef;
    
    typedef void (*MTContactCallbackFunction)(MTDeviceRef deviceId,
                                              MTTouch* touches,
                                              int32_t numTouches,
                                              double timestamp,
                                              int32_t frameNum);
    
//    typedef int32_t (*MTRegisterImageCallbackFunction)(void);
    
    double      MTAbsoluteTimeGetCurrent();

    bool        MTDeviceIsAvailable(); // is default device available
    MTDeviceRef MTDeviceCreateDefault();
    MTDeviceRef MTDeviceCreateFromDeviceID(int64_t deviceId);
    MTDeviceRef MTDeviceCreateFromService(io_service_t service);
    MTDeviceRef MTDeviceCreateFromGUID(uuid_t guid); // GUID's compared by pointer, not value!
    CFMutableArrayRef MTDeviceCreateList(); // creates for driver types 0, 1, 4, 2, 3
    
    void MTRegisterContactFrameCallback(MTDeviceRef deviceId, MTContactCallbackFunction cb);
    void MTUnregisterContactFrameCallback(MTDeviceRef deviceId, MTContactCallbackFunction cb);
    //    void MTRegisterImageCallback(MTDeviceRef deviceId,MTContactCallbackFunction cb);
    //    void MTUnregisterImageCallback(MTDeviceRef deviceId, MTRegisterImageCallbackFunction cb);
    
    void MTDeviceStart(MTDeviceRef deviceId, MTRunMode mode = LESS_VERBOSE);
    void MTDeviceStop(MTDeviceRef deviceId);
    void MTDeviceRelease(MTDeviceRef deviceId);
    bool MTDeviceIsRunning(MTDeviceRef deviceId);

    OSStatus MTDeviceGetSensorSurfaceDimensions(MTDeviceRef deviceId, int32_t* width, int32_t* height);
    OSStatus MTDeviceGetSensorDimensions(MTDeviceRef deviceId, int32_t* rows, int32_t* columns);

    char* MTGetPathStageName(MTTouchPhase touchPhase);

    bool MTDevicePowerControlSupported(MTDeviceRef dev);
    void MTDeviceSetUILocked(MTDeviceRef dev, bool state);


    bool MTDeviceIsAlive(MTDeviceRef dev) ;
    bool MTDeviceIsMTHIDDevice(MTDeviceRef dev);
    bool MTDeviceIsValid(MTDeviceRef deviceId);
    bool MTDeviceIsBuiltIn(MTDeviceRef deviceId) __attribute__ ((weak_import));	// no 10.5
    bool MTDeviceIsOpaqueSurface(MTDeviceRef deviceId);
    io_service_t MTDeviceGetService(MTDeviceRef deviceId);
    OSStatus MTDeviceGetSensorSurfaceDimensions(MTDeviceRef deviceId, int* width, int* height);
    OSStatus MTDeviceGetFamilyID(MTDeviceRef deviceId, int32_t* familyId);
    OSStatus MTDeviceGetDeviceID(MTDeviceRef deviceId, uint64_t* id) __attribute__ ((weak_import));	// no 10.5
    OSStatus MTDeviceGetDriverType(MTDeviceRef, int32_t*);
//    OSStatus MTDeviceGetActualType(MTDeviceRef, int32_t*);
    OSStatus MTDeviceGetGUID(MTDeviceRef deviceId, uuid_t* guid);

    bool MTDeviceSupportsActuation(MTDeviceRef deviceId);
    bool MTDeviceSupportsForce(MTDeviceRef deviceId);

        
#ifdef __cplusplus
}
#endif
