/*
 * ==================================================================================
 *
 *      Filename:   main.cpp
 *
 *   Description:   Main Audio Interface for Playback/Recording/Effects Loop
 *                  using portaudio
 *
 *                  Digital Oscilloscope Test
 *
 *       Version:   1.0
 *       Created:   12/24/2015
 *
 *        Author:   Ryan Foo (ryanfoo@nyu.edu)
 *       Website:   https://github.com/ryanfoo
 *
 * ==================================================================================
 */

// Global Defines
#define SAMPLE_RATE             44100           // Sampling Rate (44100 cycles/sec)
#define BUFFER_SIZE             1024            // Number of frames per buffer cycle
#define NUM_IN_CHANNELS         1               // Number of inputs
#define NUM_OUT_CHANNELS        2               // Number of outputs
#define MONO                    1               // Mono Channel
#define STEREO                  2               // Stereo Channels

// Libraries for std and portaudio
#include <stdio.h>          /* for input/output */
#include <stdlib.h>
#include <portaudio.h>      /* open-source audio io */
#include <sndfile.h>        /* for output file */
#include <string.h>         /* for memset */
#include <stdbool.h>        /* for booleans */
#include <math.h>           /* math functions */
#include <vector>         /* variable array functions */

// Sleep Routines
#include <unistd.h>

// Open GL
#include "gl_processor.h" 

// Audio Libraries
#include "OscGen.h"
#include "BiquadFilter.h"

// Data structure holding our variables
typedef struct {
    SNDFILE *outfile;       // For Output Writing
    SF_INFO sf_info;        // File info parameter
    float freq;             // Frequency
    int oct;                // Octave
    float vol;              // Volume

    bool micInputEnabled;   // Input Enable
    bool synthEnabled;      // Synth Enable

    OscGen *osc;            // Oscillator class
    BiquadFilter *bFilter;  // Biquad Filter Class
} paData;

// Port Audio Struct
PaStream *g_stream;

// Global Data Structure
paData g_data;

// Piano Roll Array
std::vector<float> midi(90);

/*
 *  Function Protoypes
 */
void initData(paData *pa);
void keyboardFunc(unsigned char, int, int);
void initialize_audio(PaStream **stream);
void stop_portAudio(PaStream **stream);

/*
 *  Name: loadHelpText()
 *  Desc: Loads Help Screen Text
 */
void loadHelpText() {
    printf("-------------------------------------\n");
    printf("Audio Terminal Waveform Oscilloscope\n");
    printf("by Ryan Foo\n");
    printf("-------------------------------------\n");
    printf("'h' - Load Help Screen Text Message\n");
    printf("'f' - Toggle Full Screen\n");
    printf("'w' - Waveform Help Text\n"); 
    printf("'e' - Filter Help Text\n");
    printf("'=' - Increase Volume\n"); 
    printf("'-' - Decrease Volume\n"); 
    printf("'<' - Decrement Frequency\n");
    printf("'>' - Increment Frequency\n");
    printf("Press caps to engage piano\n");
    printf("'q' - Quit\n");
    printf("-------------------------------------\n\n");
}

/*
 *  Name: wformSelectText()
 *  Desc: Selects waveform Screen Text
 */ 
void wformSelectText() {
    printf("-------------------------------------\n");
    printf("Choose Waveform:\n");
    printf("'0' - sine\n");
    printf("'1' - saw\n");
    printf("'2' - triangle\n");
    printf("'3' - square\n");
    printf("'4' - white noise\n");
    printf("'5' - pink noise\n");
    printf("'h' - Load Help Screen Text Message\n");
    printf("'q' - Quit\n");
    printf("-------------------------------------\n\n");
}

/*
 *  Name: filterHelpText()
 *  Desc: Filter Help Text
 */ 
void filterHelpText() {
    printf("-------------------------------------\n");
    printf("Choose Filter:\n");
    printf("'Z' - First Order LPF\n");
    printf("'X' - First Order HPF\n");
    printf("'C' - Second Order LPF\n");
    printf("'V' - Second Order HPF\n");
    printf("'B' - Second Order BPF\n");
    printf("'N' - Second Order BSF\n");
    printf("'z' - Second Order Butterworth LPF\n");
    printf("'x' - Second Order Butterworth HPF\n");
    printf("'c' - Second Order Butterworth BPF\n");
    printf("'v' - Second Order Butterworth BSF\n");
    printf("'h' - Load Help Screen Text Message\n");
    printf("'q' - Quit\n");
    printf("-------------------------------------\n\n");
}

/*
 *  Name: paCallback()
 *  Desc: callback from PortAudio
 */
static int paCallback(const void *inputBuffer, void *outputBuffer, 
        unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo *timeInfo, 
        PaStreamCallbackFlags statusFlags, void *userData) {
    // Initialize variables
    int i;
    float sample;

    // Data initialization
    float *inBuf    = (float *)inputBuffer;
    float *outBuf   = (float *)outputBuffer;
    paData *data    = (paData *)userData;

    // Allocate buffer
    memset(outBuf, 0, sizeof(float)*framesPerBuffer);
    data->osc->setFrequency(data->freq);

    // Callback loop
    for (i = 0; i < framesPerBuffer; i++) {
        // Write input to sample
        if (data->micInputEnabled) sample = inBuf[i];
    
        // Generate the oscillator
        if (data->synthEnabled) sample = data->osc->generateSample();

        // Filter Waveform
        sample = data->bFilter->processBiquad(sample);
        
        // Write sample to output
        outBuf[2*i] = sample  * data->vol;
        outBuf[2*i+1] = sample * data->vol;

        // Write to GL buffer
        g_buffer[2*i] = sample * data->vol;
        g_buffer[2*i+1] = sample * data->vol;
    }
    // Set flag
    g_ready = true;

    return paContinue;
}

/*
 *  Description: Initializes custom data
 */
void initData(paData *pa) {
    pa->freq = 0.f;
    pa->oct = 4;
    pa->micInputEnabled = false;
    pa->synthEnabled = true;

    pa->osc = new OscGen(SAMPLE_RATE);
    pa->osc->setFrequency(pa->freq);
    pa->osc->setWaveform(OscGen::SIN);
    
    pa->bFilter = new BiquadFilter(SAMPLE_RATE);
    pa->bFilter->setCutoffFrequency(5000.f);
    pa->bFilter->setQ(12.f);
    pa->bFilter->setFilterType(BiquadFilter::SO_LPF_BUTTERS);

    pa->vol = 0.5f;
}

/*
 *  Name: initialize_audio( RtAudio *dac )
 *  Desc: Initializes PortAudio with the global vars and the stream
 */
 void initialize_audio(PaStream **stream) {

    PaStreamParameters outputParameters;
    PaStreamParameters inputParameters;
    PaError err;

    /* Initialize PortAudio */
    Pa_Initialize();

    /* Set input stream parameters */
    inputParameters.device = Pa_GetDefaultInputDevice();
    inputParameters.channelCount = MONO;
    inputParameters.sampleFormat = paFloat32;
    inputParameters.suggestedLatency = 
        Pa_GetDeviceInfo( inputParameters.device )->defaultLowInputLatency;
    inputParameters.hostApiSpecificStreamInfo = NULL;

    /* Set output stream parameters */
    outputParameters.device = Pa_GetDefaultOutputDevice();
    outputParameters.channelCount = g_channels;
    outputParameters.sampleFormat = paFloat32;
    outputParameters.suggestedLatency = 
        Pa_GetDeviceInfo( outputParameters.device )->defaultLowOutputLatency;
    outputParameters.hostApiSpecificStreamInfo = NULL;

    /* Init Data */
    initData(&g_data);

    /* Open audio stream */
    err = Pa_OpenStream(&(*stream),
            &inputParameters,
            &outputParameters,
            SAMPLE_RATE, g_buffer_size, paNoFlag, 
            paCallback, &g_data);

    if (err != paNoError) {
        printf("PortAudio error: open stream: %s\n", Pa_GetErrorText(err));
    }

    /* Start audio stream */
    err = Pa_StartStream( *stream );
    if (err != paNoError) {
        printf(  "PortAudio error: start stream: %s\n", Pa_GetErrorText(err));
    }
}

/*
 *  Name: stop_portAudio(PaStream **stream)
 *  Desc: Stop, close, terminate audio stream
 */
void stop_portAudio(PaStream **stream) {
    PaError err;

    /* Stop audio stream */
    err = Pa_StopStream( *stream );
    if (err != paNoError) {
        printf(  "PortAudio error: stop stream: %s\n", Pa_GetErrorText(err));
    }
    /* Close audio stream */
    err = Pa_CloseStream(*stream);
    if (err != paNoError) {
        printf("PortAudio error: close stream: %s\n", Pa_GetErrorText(err));
    }
    /* Terminate audio stream */
    err = Pa_Terminate();
    if (err != paNoError) {
        printf("PortAudio error: terminate: %s\n", Pa_GetErrorText(err));
    }
}

/*
 *  Name: keyboardFunc( )
 *  Desc: key event
 */
void keyboardFunc(unsigned char key, int x, int y)
{
    //TODO: add function to change filter fc and q
    int octave = 16*g_data.oct;
    switch(key)
    {
        // Print Help
        case 'h':
            loadHelpText();
            break;

        // Fullscreen
        case 'f':
            if( !g_fullscreen )
            {
                g_last_width = g_width;
                g_last_height = g_height;
                glutFullScreen();
            }
            else
                glutReshapeWindow( g_last_width, g_last_height );

            g_fullscreen = !g_fullscreen;
            printf("[main]: fullscreen: %s\n", g_fullscreen ? "ON" : "OFF" );
            break;

        // Filter Help
        case 'e':
            filterHelpText();
            break;

        // Waveform Help
        case 'w':
            wformSelectText();
            break;


        // Input on
        case 'i':
            g_data.micInputEnabled = !g_data.micInputEnabled;
            break;

        // Synth
        case 'o':
            g_data.synthEnabled = !g_data.synthEnabled;
            break;

        // Volume Controls
        case '=':
            g_data.vol += 0.05f;
            break;

        case '-':
            g_data.vol -= 0.05f;
            break;

        // Waveform Controls
        case '0':
            g_data.osc->setWaveform(OscGen::SIN);
            break;

        case '1':
            g_data.osc->setWaveform(OscGen::SAW);
            break;

        case '2':
            g_data.osc->setWaveform(OscGen::TRI);
            break;

        case '3':
            g_data.osc->setWaveform(OscGen::SQR);
            break;

        case '4':
            g_data.osc->setWaveform(OscGen::WHITE);
            break;

        case '5':
            g_data.osc->setWaveform(OscGen::PINK);
            break;

        // Change Frequencies:
        case '<':
            if (g_data.oct > 0) g_data.oct--;
            break;

        case '>':
            if (g_data.oct < 7) g_data.oct++;
            break;


        // piano roll
        case 'A':
            g_data.freq = midi[4+octave];
            break;

        case 'W':
            g_data.freq = midi[5+octave];
            break;

        case 'S':
            g_data.freq = midi[6+octave];
            break;

        case 'E':
            g_data.freq = midi[7+octave];
            break;

        case 'D':
            g_data.freq = midi[8+octave];
            break;

        case 'F':
            g_data.freq = midi[9+octave];
            break;

        case 'T':
            g_data.freq = midi[10+octave];
            break;

        case 'G':
            g_data.freq = midi[11+octave];
            break;

        case 'Y':
            g_data.freq = midi[12+octave];
            break;

        case 'H':
            g_data.freq = midi[13+octave];
            break;

        case 'U':
            g_data.freq = midi[14+octave];
            break;

        case 'J':
            g_data.freq = midi[15+octave];
            break;

        case 'K':
            g_data.freq = midi[16+octave];
            break;

        // Filter options
        case 'Z':
            g_data.bFilter->setFilterType(BiquadFilter::FO_LPF);
            break;

        case 'X':
            g_data.bFilter->setFilterType(BiquadFilter::FO_HPF);
            break;

        case 'C':
            g_data.bFilter->setFilterType(BiquadFilter::SO_LPF);
            break;

        case 'V':
            g_data.bFilter->setFilterType(BiquadFilter::SO_HPF);
            break;

        case 'B':
            g_data.bFilter->setFilterType(BiquadFilter::SO_BPF);
            break;

        case 'N':
            g_data.bFilter->setFilterType(BiquadFilter::SO_BSF);
            break;

        case 'z':
            g_data.bFilter->setFilterType(BiquadFilter::SO_LPF_BUTTERS);
            break;

        case 'x':
            g_data.bFilter->setFilterType(BiquadFilter::SO_HPF_BUTTERS);
            break;

        case 'c':
            g_data.bFilter->setFilterType(BiquadFilter::SO_BPF_BUTTERS);
            break;

        case 'v':
            g_data.bFilter->setFilterType(BiquadFilter::SO_BSF_BUTTERS);
            break;

        case 'q':
            // Close Stream before exiting
            stop_portAudio(&g_stream);

            exit( 0 );
            break;
    }
}

/*
 *  Description: Main Function
 */
int main(int argc, char **argv) {

    // Create MIDI values
    midi[0] = 0;
    for (int i = 1; i < 90; i++) {
        float freq = powf(powf(2.f, 1.f/12.f), i-49)*220.f;
        midi[i] = freq;
    }

    // Initialize GLUT
    initialize_glut(argc, argv);

    // set the keyboard function - called on keyboard events
    glutKeyboardFunc( keyboardFunc );
    
    // Initialize PortAudio
    initialize_audio(&g_stream);

    // print help
    loadHelpText();

    // Wait until 'q' is pressed to stop process
    glutMainLoop();

    return EXIT_SUCCESS;
}
