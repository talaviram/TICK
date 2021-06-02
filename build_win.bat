REM "%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin\MSBuild.exe" ./JUCE/extras/Projucer/Builds/VisualStudio2019/Projucer.sln
REM "./JUCE/extras/Projucer/Builds/VisualStudio2019/x64/Debug/App/Projucer.exe" --resave ./TICK.jucer

echo Building GPL formats
set CL=/DJUCE_DISPLAY_SPLASH_SCREEN#0
call "%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin\MSBuild.exe" ./Builds/VisualStudio2019/TICK_SharedCode.vcxproj /p:Configuration=Release /p:platform=x64 /t:rebuild /m
call "%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin\MSBuild.exe" ./Builds/VisualStudio2019/TICK_StandalonePlugin.vcxproj /p:Configuration=Release /p:platform=x64 /t:rebuild /m
call "%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin\MSBuild.exe" ./Builds/VisualStudio2019/TICK_VST3.vcxproj /p:Configuration=Release /p:platform=x64 /t:rebuild /m

echo Building non-GPL with Splash
set CL=/DJUCE_DISPLAY_SPLASH_SCREEN#1
call "%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin\MSBuild.exe" ./Builds/VisualStudio2019/TICK_SharedCode.vcxproj /p:Configuration=Release /t:rebuild /p:platform=x64 /m
call "%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin\MSBuild.exe" ./Builds/VisualStudio2019/TICK_AAX.vcxproj /p:Configuration=Release /p:platform=x64 /t:rebuild /m
call "%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin\MSBuild.exe" ./Builds/VisualStudio2019/TICK_VST.vcxproj /p:Configuration=Release /p:platform=x64 /t:rebuild /m
