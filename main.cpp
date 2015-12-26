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

// Sleep Routines
#include <unistd.h>

// Open GL
#include "gl_processor.h" 

// Data structure holding our variables
typedef struct {
    SNDFILE *outfile;       // For Output Writing
    SF_INFO sf_info;        // File info parameter
} paData;

// Port Audio Struct
PaStream *g_stream;

/*
 *  Function Protoypes
 */
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
    // paData *data    = (paData *)userData;

    // Allocate buffer
    memset(outBuf, 0, sizeof(float)*framesPerBuffer);
    
    float phs_incr = 440.*2*3.14/44100;
    static float phs = 0;

    // Callback loop
    for (i = 0; i < framesPerBuffer; i++) {
        // Write input to sample
        // sample = inBuf[i];
        
        sample = sinf(phs);
        if (sample > 0) sample = 1;
        else sample = -1;
        phs += phs_incr;

        if (phs >= 2*3.14) phs = 0;

        // Write sample to output
        outBuf[2*i] = sample;
        outBuf[2*i+1] = sample;

        // Write to GL buffer
        g_buffer[2*i] = sample;
        g_buffer[2*i+1] = sample;
    }
    // Set flag
    g_ready = true;

    return paContinue;
}

/*
 *  Description: Initializes custom data
 */
void initData(paData *pa) {
    //TODO: put initialization calls here
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

    /* Open audio stream */
    err = Pa_OpenStream( &(*stream),
            &inputParameters,
            &outputParameters,
            SAMPLE_RATE, g_buffer_size, paNoFlag, 
            paCallback, NULL );

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
    //printf("key: %c\n", key);
    switch( key )
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
