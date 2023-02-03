#!/bin/sh
echo "Building GPL formats"
cmake -B ./build -G "Xcode" -DCOPY_AFTER_BUILD="FALSE" -DCMAKE_CXX_FLAGS="${CMAKE_CXX_FLAGS} -DJUCE_DISPLAY_SPLASH_SCREEN=0"
cmake --build ./build --clean-first --target TICK_Standalone TICK_AUv3 TICK_AU TICK_VST3 --config RelWithDebInfo
echo "Building non-GPL with Splash"
cmake -B ./build -G "Xcode" -DCOPY_AFTER_BUILD="FALSE" -DCMAKE_CXX_FLAGS="${CMAKE_CXX_FLAGS} -DJUCE_DISPLAY_SPLASH_SCREEN=1"
cmake --build ./build --target TICK_VST TICK_AAX --config RelWithDebInfo

echo Copying built binaries...
mkdir -p result/mac
# Projucer locations
# TODO: add non-GPL which should be built with different flags
cp -R -L ./build/TICK_artefacts/RelWithDebInfo/AAX/TICK.aaxplugin ./result/mac
cp -R -L ./build/TICK_artefacts/RelWithDebInfo/Standalone/TICK.app ./result/mac
cp -R -L ./build/TICK_artefacts/RelWithDebInfo/AU/TICK.component ./result/mac
cp -R -L ./build/TICK_artefacts/RelWithDebInfo/VST/TICK.vst ./result/mac
cp -R -L ./build/TICK_artefacts/RelWithDebInfo/VST3/TICK.vst3 ./result/mac

echo Apple codesign...
./private/codesign.sh ./result/mac/TICK.aaxplugin
./private/codesign.sh ./result/mac/TICK.app
./private/codesign.sh ./result/mac/TICK.component
./private/codesign.sh ./result/mac/TICK.vst
./private/codesign.sh ./result/mac/TICK.vst3

echo Sign AAX...
python3 ./private/sign_aax.py ./result/mac/TICK.aaxplugin

echo Create Installer...
./private/packages.sh ./Installer/TICK.pkgproj
echo Notarize Installer
./private/notarize.sh ./Installer/build/TICK.pkg