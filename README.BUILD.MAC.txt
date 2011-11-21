The minimum requirements to build The Warzone Model Importer Tool are System 10.6 and Xcode 3.2.x. and the Qt 4.7 moc.


If you do not have Xcode 3.2.x you can get it for free at Apple's website.
http://developer.apple.com/technology/xcode.html
You will need a free ADC Membership to download Xcode.

If you do not the Qt 4.7 moc you can get it at the Qt website.
http://qt.nokia.com/downloads/qt-for-open-source-cpp-development-on-mac-os-x

To build the tool, just run the following command:
	xcodebuild -project Warzone.xcodeproj -target "Warzone Model Importer Tool" -configuration Release

There are two build configurations available.  'Release' is compiled
normally, while 'Debug' is suited for use with a debugger.
