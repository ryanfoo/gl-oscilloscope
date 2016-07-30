/*
 * ==================================================================================
 *
 *      Filename:   BiquadFilter.h
 *
 *   Description:   Biquad Filter Implementation
 *  
 *       Version:   1.0
 *       Created:   12/26/2015
 *
 *        Author:   Ryan Foo (ryanfoo@nyu.edu)
 *       Website:   https://github.com/ryanfoo
 *
 * ==================================================================================
 */
#ifndef BIQUADFILTER_H
#define BIQUADFILTER_H

#include <math.h>
#include <float.h>

class BiquadFilter {
public:
    // Filter Type
    enum FILTER {
        FO_LPF = 0,
        FO_HPF = 1,
        SO_LPF = 3,
        SO_HPF = 4,
        SO_BPF = 5,
        SO_BSF = 6,
        SO_LPF_BUTTERS = 7,
        SO_HPF_BUTTERS = 8,
        SO_BPF_BUTTERS = 9,
        SO_BSF_BUTTERS = 10,
    };

    // Initializations
    BiquadFilter() { srate = 44100.f; fc = 0; g = 1; x1 = x2 = y1 = y2 = 0; };
    BiquadFilter(float _srate) { srate = _srate; fc = 0; g = 1; x1 = x2 = y1 = y2 = 0; };
    ~BiquadFilter();

    // Filter Setup
    void setFilterGain(float gain) { g = gain; };
    void setCutoffFrequency(float _fc) { fc = _fc; };
    void setQ(float _q) { q = _q; };
    void setFilterType(float _filter) { 
        if (filter == _filter) return;
        filter = _filter;
        configureFilter();
    };

    // Configure Filter
    void configureFilter() {
        switch (filter) {
            case FO_LPF: {
                float phs = 2.f*M_PI*fc/srate;
                // Gamma:
                float gamma = cosf(phs)/(1+sinf(phs));
                // Alpha:
                float alpha = (1-gamma)/2.f;

                // Coefs:
                a0 = alpha;
                a1 = alpha;
                a2 = 0;
                b1 = -gamma;
                b2 = 0;
                break;
            }
            case FO_HPF: {
                float phs = 2.f*M_PI*fc/srate;
                // Gamma:
                float gamma = cosf(phs)/(1+sinf(phs));
                // Alpha:
                float alpha = (1+gamma)/2.f;

                // Coefs:
                a0 = alpha;
                a1 = -alpha;
                a2 = 0;
                b1 = -gamma;
                b2 = 0;
                break;
            }
            case SO_LPF: {
                float phs = 2.f*M_PI*fc/srate;
                float d = 1.f/q;
                // Beta:
                float beta_num = 1.f - ((d/2.f)*(sinf(phs)));
                float beta_den = 1.f + ((d/2.f)*(sinf(phs)));
                float beta = 0.5f * (beta_num/beta_den);
                // Gamma:
                float gamma = (0.5f + beta) * cosf(phs);
                // Alpha:
                float alpha = (0.5f + beta - gamma)/2.f;

                // Coefs:
                a0 = alpha;
                a1 = alpha*2.f;
                a2 = alpha;
                b1 = -2*gamma;
                b2 = 2*beta;
                break;
            }
            case SO_HPF: {
                float phs = 2.f*M_PI*fc/srate;
                float d = 1.f/q;
                // Beta:
                float beta_num = 1.f - ((d/2.f)*(sinf(phs)));
                float beta_den = 1.f + ((d/2.f)*(sinf(phs)));
                float beta = 0.5f * (beta_num/beta_den);
                // Gamma:
                float gamma = (0.5f + beta) * cosf(phs);
                // Alpha:
                float alpha = (0.5f + beta + gamma)/2.f;

                // Coefs:
                a0 = alpha;
                a1 = -alpha*2.f;
                a2 = alpha;
                b1 = -2*gamma;
                b2 = 2*beta;
                break;
            }
            case SO_BPF: {
                float phs = 2.f*M_PI*fc/srate;
                // Beta:
                float beta_num = 1.f - tanf(phs/(2.f*q));
                float beta_den = 1.f + tanf(phs/(2.f*q));
                float beta = 0.5f * (beta_num/beta_den);
                // Gamma:
                float gamma = (0.5f + beta) * cosf(phs);
                // Alpha:
                float alpha = (0.5f - beta);

                // Coefs:
                a0 = alpha;
                a1 = 0;
                a2 = -alpha;
                b1 = -2*gamma;
                b2 = 2*beta;
                break;
            }
            case SO_BSF: {
                float phs = 2.f*M_PI*fc/srate;
                // Beta:
                float beta_num = 1.f - tanf(phs/(2.f*q));
                float beta_den = 1.f + tanf(phs/(2.f*q));
                float beta = 0.5f * (beta_num/beta_den);
                // Gamma:
                float gamma = (0.5f + beta) * cosf(phs);
                // Alpha:
                float alpha = (0.5f + beta);

                // Coefs:
                a0 = alpha;
                a1 = -2*gamma;
                a2 = alpha;
                b1 = -2*gamma;
                b2 = 2*beta;
                break;
            }
            case SO_LPF_BUTTERS: {
                float C = 1/(tanf(M_PI*fc/srate));
                // Coefs:
                a0 = 1/(1+(sqrt(2)*C)+powf(C,2));
                a1 = 2*a0;
                a2 = a0;
                b1 = (2*a0)*(1-powf(C,2));
                b2 = a0*(1-(sqrt(2)*C)+powf(C,2));
                break;
            }
            case SO_HPF_BUTTERS: {
                float C = tanf(M_PI*fc/srate);
                // Coefs:
                a0 = 1/(1+(sqrt(2)*C)+powf(C,2));
                a1 = -2*a0;
                a2 = a0;
                b1 = (2*a0)*(powf(C,2)-1);
                b2 = a0*(1-(sqrt(2)*C)+powf(C,2));
                break;
            }
            case SO_BPF_BUTTERS: {
                float BW = fc/q;                                         
                float C = 1/(tanf(M_PI*fc*BW/srate));
                float D = 2*cosf(2*M_PI*fc/srate);
                // Coefs:
                a0 = 1/(1+C);
                a1 = 0;
                a2 = -a0;
                b1 = -a0*(C*D); 
                b2 = a0*(C-1);
                break;
            }
            case SO_BSF_BUTTERS: {
                float BW = fc/q;                                         
                float C = tanf(M_PI*fc*BW/srate);
                float D = 2*cosf(2*M_PI*fc/srate);
                // Coefs:
                a0 = 1/(1+C);
                a1 = -a0*D;
                a2 = a0;
                b1 = -a0*D; 
                b2 = a0*(1-C);
                break;
            }

        };
    };

    // Biquad Processing Block
    float processBiquad(float xn) {
        // Difference Equation:
        float yn = (g*((a0*xn) + (a1*x1) + (a2*x2))) - (b1*y1) - (b2*y2);
        // underflow check
        if (yn > 0.f && yn < FLT_MIN) yn = 0;
        if (yn < 0.f && yn > FLT_MIN) yn = 0;

        // Takes pop out when no input
        if (xn == 0) { yn = 0; y1 = 0; y2 = 0; }

        y2 = y1;
        y1 = yn;
        x2 = x1;
        x1 = xn;
        
        return (yn + xn)/2;
    };

private:
    // Y delays
    float y1, y2;
    // x delays
    float x1, x2;
    // filter gain
    float g;

    // coefficients
    float a0, a1, a2, b1, b2;

    // Variables
    float srate;
    float fc;
    float q;
    int filter;
};

#endif // BIQUADFILTER_H
