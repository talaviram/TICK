#!/bin/sh -e
pgrep -x AudioComponentRegistrar >/dev/null && killall -9 AudioComponentRegistrar; echo "killed AudioComponentRegistrar" || echo "AudioComponentRegistrar Process not found"
