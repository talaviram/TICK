TICK - Professional Open-Source Metronome
=========================================

CURRENT STATE:
--------------
Since I do this in my spare (which become more limited with COVID19, small kids), This is still WIP.

I do it too slowly. (mostly fiddeling with responsive UI to be good enough for mobile and desktop).
But the basic metronome does work.


Background:
-----------
If you ever worked with more than one host (DAW), you've discovered each one got their own built-in metronome.
Those greatly differ in available sounds and routing cabilities.
While many hosts allow sending metronome as MIDI, this requires tidious set-up.

TICK is tailored made to make a small portable Metronome plug-in.
It can also work in standalone mode, meaning it can have agnostic time to the host itself.


Features:
---------
- Sample-Accurate Metronome.
- Customizable - You choose exactly how your metronome sounds. Including ability to do basic edits inside Tick.
- Portable - TICK is self-contained. As a plug-in, your audio samples are saved in the preset that can easily exported and imported.
- Low Pass Filter - avoid bleeds while recording.
- True Cross-Platform - TICK ~~is~~ will be available as an audio plug-in or a standalone app for Mac, Windows, Linux, iOS & Android as an app.
- Open-Source.
- Open-Standards - Tick preset format is a zip archive with plain WAVE files (containing metadata) along a simple readable XML.

```
<?xml version="1.0" encoding="UTF-8"?>
<TICK_SETTINGS presetName="Nerd Readable Format" numOfTicks="2" useHostTransport="0">
  <BEAT index="0" tickId="1" gain="1.0"/>
  ...
  <BEAT index="63" tickId="0" gain="1.0"/>
  <TRANSPORT isPlaying="1" numerator="4" denumerator="4" bpm="120"/>
</TICK_SETTINGS>
```


Upcoming
--------
- Finish all the corner cases.
- Clean/refactor ugly code.
- Refactor to background thread for some nasty file operations.
- Optimize on mobile devices. quit app if metronome isn't playing.
- Tap, make tap great again!
- Tomper (nice idea to be explained in the future)
- Ableton Link.
- Desktop drag-and-drop support, drag-and-drop to reorganize samples.
- Full Screen / Tablet UI, use available real-estate properly.


License and Binaries
--------------------
TICK uses [JUCE](https://www.juce.com) framework.

[JUX](https://github.com/talaviram/jux) -
During TICK, I've started seeing there are many missing UX elements that would be nice to have with JUCE. so I've separated those to JUX.

TICK itself is MIT.
JUCE and VST3 are GPL/dual-licensed.
