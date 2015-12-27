# gl-oscilloscope
PortAudio-OpenGL Waveform Drawer for Testing Audio Algorithms

History:
    I've built this for the purposes of testing my audio algorithms and seeing the waveform it produces. This only works on Mac OS X as of right now. I'll be adding example audio algorithms as time goes by, feel free to use this waveform generator and any audio files I add.

How to install:

1.  Install brew. Go to this website to install -> http://brew.sh/
2.  Install portaudio. 
        -> brew install portaudio

Audio Algorithms:

    OscGen.h
        1. Generates Waveform Oscillation
        2. Pick between Sine, Sawtooth, Triangle, Square, White Noise, and Pink Noise waveforms.

    BiquadFilter.h
        1. An Infinite Impulse Response (IIR) Filter Implementation. 
           Implementations thanks to Will Pirkle's Designing Audio Effect Plug-ins in C++, Ch. 6
        2. Pick between:
           First Order Lowpass+Highpass Filters, 
           Second Order Lowpass+Highpass+Bandpass+Bandshelf Filters, 
           Second Order Butterworth Lowpass+Highpass+Bandpass+Bandshelf Filters
        3. TO BE ADDED: More IIR Filter Implementations, Allow user to switch between Filters/Cutoff Frequencies/Q
