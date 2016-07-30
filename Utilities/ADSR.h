/*
 * ==================================================================================
 *
 *      Filename:   ADSR.h
 *      
 *   Description:   ADSR (attack decay sustain release) envelope Implementation
 *  
 *       Version:   1.0
 *       Created:   12/30/2015
 *
 *        Author:   Ryan Foo (ryanfoo@nyu.edu)
 *       Website:   https://github.com/ryanfoo
 *
 * ==================================================================================
 */
#ifndef ADSR_H
#define ADSR_H

class ADSR {
public:
    // Envelope Status
    enum {
        ATTACK,
        DECAY,
        SUSTAIN,
        RELEASE,
        IDLE
    };

    // Initializations
    ADSR() {
        srate = 44100.f;
        target = cur = 0;
        aRate = dRate = 0.001;
        rRate - 0.005;
        rTime = -1.0;
        sustain = 0.5;
        state = IDLE;
    };
    ADSR(float _srate) {
        srate = _srate;
        target = cur = 0;
        aRate = dRate = 0.001;
        rRate - 0.005;
        rTime = -1.0;
        sustain = 0.5;
        state = IDLE;
    };
    ~ADSR();

    void keyOn() {
        if (target <= 0.0) target = 1.0;
        state = ATTACK;
    };

    void keyOff() {
        target = 0.0;
        state = RELEASE;
        if (rTime > 0) rRate = cur / (rTime * srate);
    };

    void setAttackRate(float rate) {
        aRate =rate;
    };

    void setAttackTarget(float _target) {
        target = _target;
    };

    void setDecayRate(float rate) {
        dRate = rate;
    };

    void setSustain(float level) {
        sustain = level;
    };

    void setRelease(float rate) {
        rRate = rate;
        rTime = -1;
    };

    void setAttackTime(float time) {
        aRate = 1.0 / (time * srate);
    };

    void setDecayTime(float time) {
        dRate = (1 - sustain) / (time * srate);
    };

    void setReleaseTime(float time) {
        rRate = sustain / (time * srate);
        rTime = time;
    };

    void setAllTimes(float atk, float dcy, float sus, float rel) {
        setAttackTime(atk);
        setDecayTime(dcy);
        setSustain(sus);
        setReleaseTime(rel);
    };

    void setTarget(float _target) {
        target = _target;
        setSustain(target);
        if (cur < target) state = ATTACK;
        if (cur > target) state = DECAY;
    };

    void setValue(float val) {
        state = SUSTAIN;
        target = val;
        cur = val;
        setSustain(val);
    };

    int getState() {
        return state;
    }

    float processEnvelope() {
        switch (state) {
            case ATTACK: {
                cur += aRate;
                if (cur >= target) {
                    cur = target;
                    target = sustain;
                    state = DECAY;
                }
                break;
            }
            
            case DECAY: {
                if (cur > sustain) {
                    cur -= dRate;
                    if (cur <= sustain) {
                        cur = sustain;
                        state = SUSTAIN;
                    }
                }
                else {
                    cur += dRate;
                    if (cur >= sustain) {
                        cur = sustain;
                        state = SUSTAIN;
                    }
                }
                break;
            }
            
            case RELEASE: {
                cur -= rRate;
                if (cur <= 0) {
                    cur = 0;
                    state = IDLE;
                }
                break;
            }
            
        }
    
        return cur;
    };

private:
    int state;
    float srate, cur, target, aRate, dRate, rRate, rTime, sustain;
};

#endif // ADSR_H
