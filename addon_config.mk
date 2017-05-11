meta:
	ADDON_NAME = ofxTouchPad
	ADDON_DESCRIPTION = OSX Multitouch driver.
	ADDON_AUTHOR = @bakercp
	ADDON_TAGS = "touchpad" "multitouch"
	ADDON_URL = https://github.com/bakercp/ofxTouchPad

common:
	ADDON_DEPENDENCIES = ofxPointer
	ADDON_LDFLAGS = -F/System/Library/PrivateFrameworks -framework MultitouchSupport
