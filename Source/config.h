/*
* config.h for libsamplerate to be used with JUCE.
* this removes the need of generated ones.
*/
#pragma once

// TODO - find a way to not hardcode PACKAGE and VERSION

/* Name of package */
#define PACKAGE "libsamplerate"

/* Version number of package */
#define VERSION "0.1.9"

// TODO - really check those
/* Target processor clips on negative float to int conversion. */
#define CPU_CLIPS_NEGATIVE 1

/* Target processor clips on positive float to int conversion. */
#define CPU_CLIPS_POSITIVE 1

/* Target processor is little endian. */
#if JUCE_LITTLE_ENDIAN
#define CPU_IS_LITTLE_ENDIAN
#else
/* Target processor is big endian. */
#define CPU_IS_BIG_ENDIAN
#endif

// I didn't see much cases where 'alarm' isn't available.
/* Define to 1 if you have the `alarm' function. */
#define HAVE_ALARM

/* Define to 1 if you have the <alsa/asoundlib.h> header file. */
#if (JUCE_LINUX && JUCE_ALASA)
#define HAVE_ALSA
#endif

#if JUCE_DSP_USE_STATIC_FFTW || JUCE_DSP_USE_SHARED_FFTW
/* Set to 1 if you have libfftw3. */
#define HAVE_FFTW3
#endif

// JUCE currently support C++11 or greater
/* Define if you have C99's lrint function. */
#define HAVE_LRINT 1

/* Define if you have C99's lrintf function. */
#define HAVE_LRINTF 1

/* Define if you have signal SIGALRM. */
#define HAVE_SIGALRM 1

/* Define to 1 if you have the `signal' function. */
#define HAVE_SIGNAL 1

/* JUCE don't have libsndfile. */
#define HAVE_SNDFILE 0

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <sys/times.h> header file. */
#define HAVE_SYS_TIMES_H 1

/* Set to 1 if compiling for Win32 */
#if JUCE_WINDOWS
#define OS_IS_WIN32 1
#endif

/* The size of `int', as computed by sizeof. */
#define SIZEOF_INT __SIZEOF_INT__

/* The size of `long', as computed by sizeof. */
#define SIZEOF_LONG __SIZEOF_LONG__
