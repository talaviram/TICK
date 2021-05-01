#!/bin/sh
xcodebuild -project ./JUCE/extras/Projucer/Builds/MacOSX/Projucer.xcodeproj || { echo 'Projucer build failed' ; exit 1; }
./JUCE/extras/Projucer/Builds/MacOSX/build/Debug/Projucer.app/Contents/MacOS/Projucer --resave ./TICK.jucer || { echo 'JUCE -> Xcode Exporting failed' ; exit 1; }
xcodebuild -project ./Builds/MacOSX/TICK.xcodeproj -configuration Release || { echo 'Build failed!' ; exit 1; }
echo Copying built binaries...
mkdir -p result/mac
# Projucer locations
# TODO: add non-GPL which should be built with different flags
# cp -R -L ./Builds/MacOSX/build/Release/TICK.aax ./result/mac
cp -R -L ./Builds/MacOSX/build/Release/TICK.app ./result/mac
cp -R -L ./Builds/MacOSX/build/Release/TICK.component ./result/mac
# cp -R -L ./Builds/MacOSX/build/Release/TICK.vst ./result/mac
cp -R -L ./Builds/MacOSX/build/Release/TICK.vst3 ./result/mac
echo Create Installer...
packagesbuild ./Installer/TICK.pkgproj