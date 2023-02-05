#!/bin/sh
TICK_PATH=$('pwd')

echo "Building GPL formats"
cmake -B ./build -DCOPY_AFTER_BUILD="FALSE" -DCMAKE_CXX_FLAGS="${CMAKE_CXX_FLAGS} -DJUCE_DISPLAY_SPLASH_SCREEN=0"
cmake --build ./build --clean-first --target TICK_Standalone TICK_VST3 --config RelWithDebInfo
echo "Building non-GPL with Splash"
cmake -B ./build -DCOPY_AFTER_BUILD="FALSE" -DCMAKE_CXX_FLAGS="${CMAKE_CXX_FLAGS} -DJUCE_DISPLAY_SPLASH_SCREEN=1"
cmake --build ./build --target TICK_VST --config RelWithDebInfo

cd $TICK_PATH
python3 ./Installer/make_linux_installer.py .
