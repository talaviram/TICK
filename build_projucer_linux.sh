#!/bin/sh
TICK_PATH=$('pwd')
cd ./JUCE/extras/Projucer/Builds/LinuxMakefile/ && make && pwd
cd $TICK_PATH
./JUCE/extras/Projucer/Builds/LinuxMakefile/build/Projucer ./TICK.jucer
