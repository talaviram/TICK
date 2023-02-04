@echo off
echo Building GPL formats
cmake -B ./build -DCOPY_AFTER_BUILD="FALSE" -DCMAKE_CXX_FLAGS="/DJUCE_DISPLAY_SPLASH_SCREEN=0"
cmake --build ./build --clean-first --target TICK TICK_Standalone TICK_VST3 --config RelWithDebInfo
echo Building non-GPL with Splash
cmake -B ./build -DCOPY_AFTER_BUILD="FALSE" -DCMAKE_CXX_FLAGS="/DJUCE_DISPLAY_SPLASH_SCREEN=1"
cmake --build ./build --target TICK TICK_VST TICK_AAX --config RelWithDebInfo

@REM echo Sign AAX
@REM python3 .\private\sign_aax.py ".\TICK_artefacts\RelWithDebInfo\AAX\TICK.aaxplugin\Contents\x64\TICK.aaxplugin"
