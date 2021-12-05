#!/bin/sh
TICK_PATH=$('pwd')
cd ./JUCE/extras/Projucer/Builds/LinuxMakefile/ && make && pwd
cd $TICK_PATH
./JUCE/extras/Projucer/Builds/LinuxMakefile/build/Projucer --resave ./TICK.jucer

cd ./Builds/LinuxMakefile/ && make clean && make CONFIG=Release
cd $TICK_PATH
python3 ./Installer/make_linux_installer.py .
