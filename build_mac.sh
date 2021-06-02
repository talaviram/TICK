#!/bin/sh
xcodebuild -project ./JUCE/extras/Projucer/Builds/MacOSX/Projucer.xcodeproj || { echo 'Projucer build failed' ; exit 1; }
./JUCE/extras/Projucer/Builds/MacOSX/build/Debug/Projucer.app/Contents/MacOS/Projucer --resave ./TICK.jucer || { echo 'JUCE -> Xcode Exporting failed' ; exit 1; }

echo "Building GPL formats"
xcodebuild -project ./Builds/MacOSX/TICK.xcodeproj -configuration Release -target "TICK - AU" || { echo 'Build failed!' ; exit 1; }
xcodebuild -project ./Builds/MacOSX/TICK.xcodeproj -configuration Release -target "TICK - Standalone Plugin" || { echo 'Build failed!' ; exit 1; }
xcodebuild -project ./Builds/MacOSX/TICK.xcodeproj -configuration Release -target "TICK - VST3" || { echo 'Build failed!' ; exit 1; }

echo "Building non-GPL with Splash"
xcodebuild ARCHS=x86_64 ONLY_ACTIVE_ARCH=NO -project ./Builds/MacOSX/TICK.xcodeproj -configuration Release -target "TICK - AAX" GCC_PREPROCESSOR_DEFINITIONS='$GCC_PREPROCESSOR_DEFINITIONS JUCE_DISPLAY_SPLASH_SCREEN=1' || { echo 'Build failed!' ; exit 1; }
xcodebuild -project ./Builds/MacOSX/TICK.xcodeproj -configuration Release -target "TICK - VST" GCC_PREPROCESSOR_DEFINITIONS='$GCC_PREPROCESSOR_DEFINITIONS JUCE_DISPLAY_SPLASH_SCREEN=1' || { echo 'Build failed!' ; exit 1; }

# builds all targets (not used to add non-GPL splash)
# xcodebuild -project ./Builds/MacOSX/TICK.xcodeproj -configuration Release || { echo 'Build failed!' ; exit 1; }

echo Copying built binaries...
mkdir -p result/mac
# Projucer locations
# TODO: add non-GPL which should be built with different flags
# cp -R -L ./Builds/MacOSX/build/Release/TICK.aax ./result/mac
cp -R -L ./Builds/MacOSX/build/Release/TICK.app ./result/mac
cp -R -L ./Builds/MacOSX/build/Release/TICK.component ./result/mac
cp -R -L ./Builds/MacOSX/build/Release/TICK.vst ./result/mac
cp -R -L ./Builds/MacOSX/build/Release/TICK.vst3 ./result/mac
cp -R -L ./Builds/MacOSX/build/Release/TICK.aaxplugin ./result/mac

echo Create Installer...
packagesbuild ./Installer/TICK.pkgproj