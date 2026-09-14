#pragma once
#include <cmath>

namespace alienmobile {
// Limit work between draws. Each tick still uses the original fixed dt;
// overload slows wall-clock progress instead of creating catch-up feedback.
class FrameStepBudget {
public:
    explicit FrameStepBudget(double seconds, unsigned maxSteps=8)
        : _seconds(seconds), _maxSteps(maxSteps) {}
    bool canStep(double pending,double fixedStep,double elapsed) const {
        return pending>=fixedStep && _steps<_maxSteps && (_steps==0 || elapsed<_seconds);
    }
    void didStep() { ++_steps; }
    static double discardBacklog(double pending,double fixedStep) {
        return pending>=fixedStep ? std::fmod(pending,fixedStep) : pending;
    }
private:
    double _seconds;
    unsigned _maxSteps, _steps=0;
};
}
