"%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin\MSBuild.exe" ./JUCE/extras/Projucer/Builds/VisualStudio2019/Projucer.sln
"./JUCE/extras/Projucer/Builds/VisualStudio2019/x64/Debug/App/Projucer.exe" --resave ./TICK.jucer

"%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin\MSBuild.exe" ./Builds/VisualStudio2019/TICK.sln /p:Configuration=Release
