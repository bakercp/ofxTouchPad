meta:
	ADDON_NAME = ofxTouchPad
	ADDON_DESCRIPTION = Multitouch support for touchpads.
	ADDON_AUTHOR = @bakercp
	ADDON_TAGS = "touchpad" "multitouch"
	ADDON_URL = https://github.com/bakercp/ofxTouchPad

osx:
	ADDON_LDFLAGS = -F/System/Library/PrivateFrameworks -framework MultitouchSupport
