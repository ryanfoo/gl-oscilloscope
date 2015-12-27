/*
 * ==================================================================================
 *
 *      Filename:   OscGen.h
 *
 *   Description:   Oscillator Waveform Generator
 *  
 *       Version:   1.0
 *       Created:   12/26/2015
 *
 *        Author:   Ryan Foo (ryanfoo@nyu.edu)
 *       Website:   https://github.com/ryanfoo
 *
 * ==================================================================================
 */

#ifndef OSCGEN_H
#define OSCGEN_H

#include <math.h>

class OscGen {
public:
    // Waveform Type
    enum WAVEFORM {
        SIN = 0,            // Sine Wave
        SAW = 1,            // Sawtooth Wave
        TRI = 2,            // Triangle Wave
        SQR = 3,            // Square Wave
        WHITE = 4,          // White Noise 
        PINK = 5,           // Pink Noise
    };

    // Initializations
    OscGen () { 
        srate = 44100.f; 
        freq = 0.f; 
        phs = 0.f; 
        phs_incr = 2*M_PI*freq/srate; 
        if (freq != 0) T = srate/freq;
    };
    OscGen (float _srate) { 
        srate = _srate; 
        freq = 0;
        phs = 0; 
        phs_incr = 2*M_PI*freq/srate; 
        T = srate/freq;
    };
    ~OscGen();

    // Setters
    void setFrequency(float _freq) { freq = _freq; phs_incr = 2*M_PI*freq/srate; T = srate/freq; };
    void setWaveform(int _wform) { waveform = _wform; };

    int getWaveform() { return waveform; };

    // Phase Wrapper
    float wrapPhase(float _phs) {
        if (phs >= (2*M_PI)) phs -= (2*M_PI);
        return phs;
    };

    // Waveform Selector
    float generateSample() {
        //TODO: silence when switching waveforms
        float sample = 0;
        static float saw_sample;

        switch (waveform) {
            case SIN: {
                sample = sinf(phs);
         
                phs += phs_incr;
                phs = wrapPhase(phs);
                break;
            }

            case SAW: {
                saw_sample += 2./T;
                if (saw_sample >= 1.f) saw_sample -= 2.f;

                sample = saw_sample;
                break;
            }

            case TRI: {
                saw_sample += 2./T;
                sample = fabsf(saw_sample) * 2.f - 1.f;

                if (saw_sample >= 1.f) saw_sample -= 2.f;
                break;
            }

            case SQR: {
                sample = sinf(phs);
                if (sample > 0) sample = 1;
                else if (sample < 0) sample = -1;
        
                phs += phs_incr;
                phs = wrapPhase(phs);
                break;
            }

            case WHITE: {
                float R1 = (float) rand() / (float) RAND_MAX;
                float R2 = (float) rand() / (float) RAND_MAX;

                sample = (float) sqrt( -2.0f * log( R1 )) * cos( 2.0f * M_PI * R2 ) / 2.f;
                break;
            }

            case PINK: {
                static const float RMI2 = 2.0 / float(RAND_MAX); // + 1.0; // change for range [0,1)
                static const float offset = A[0] + A[1] + A[2];

                 // unrolled loop
                float temp = float(rand());
                state[0] = P[0] * (state[0] - temp) + temp;
                temp = float(rand());
                state[1] = P[1] * (state[1] - temp) + temp;
                temp = float(rand());        
                state[2] = P[2] * (state[2] - temp) + temp;
                sample = ((A[0]*state[0] + A[1]*state[1] + A[2]*state[2])*RMI2 - offset)*2.f;
                break;
            }


        };

        return sample;
    };

private:
    float freq, srate, phs, phs_incr, T;
    int waveform;

    float state[3];
    const float A[3] = { 0.02109238, 0.07113478, 0.68873558 }; // rescaled by (1+P)/(1-P)
    const float P[3] = { 0.3190,  0.7756,  0.9613  };
};

#endif  // OSCGEN_H
