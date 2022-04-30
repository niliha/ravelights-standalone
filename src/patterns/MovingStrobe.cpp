#include "MovingStrobe.hpp"

#include <set>

namespace Pattern {
template <typename T> int sgn(T val) { return (T(0) < val) - (val < T(0)); }

MovingStrobe::MovingStrobe(double p_bigstrobe, double p_pause, double p_thin)
    : AbstractPattern(), bigStrobeProb_(p_bigstrobe), pauseProb_(p_pause), thinningProb_(p_thin) {
    reset();
}

void MovingStrobe::reset() {
    distortionProb_ = uniformDist_005_02_(randomGenerator_);
    light = random(lightCount_);
    frame = 0;
    pos = max(std::lround(abs(normalDist_0_1_(randomGenerator_) * pixelCount_)), (long)0);
    error = 0;
    speed = random(1, 5 + 1);
    length = random(5, 30 + 1);
    maxFrameCount_ = random(5, 25 + 1);
    errorSpeed_ = max(normalDist_2_05_(randomGenerator_), (double)1);

    // special mode : bigstrobe
    doBigStrobe_ = false;
    if (sampleBernoulli(bigStrobeProb_)) {
        doBigStrobe_ = true;
        maxFrameCount_ = random(2, 10);
    }
    // special mode : thinned LED
    doThinning_ = false;
    thinningAmount_ = 10;
    if (sampleBernoulli(thinningProb_)) {
        doThinning_ = true;
    }
    // special mode : pause
    doPause_ = false;
    if (sampleBernoulli(pauseProb_)) {
        doPause_ = true;
    }
}

unsigned MovingStrobe::perform(std::vector<CRGB> &leds, CRGB color) {
    FastLED.clearData();
    frame++;
    // see if animation is finished
    if (frame > maxFrameCount_) {
        reset();
    }
    if (doPause_) {
        return showAndMeasureRemainingDuration(1000 / 30);
    }
    // apply direction
    // !possibly broken
    int direction = 1;
    if (doRandomDirection_ && !sampleBernoulli(0.5)) {
        direction = -1;
    }
    // get intensity
    double intens = min((double)1, abs(std::sin(frame * sinFactor_)) + 0.1);
    // update position
    pos = direction * std::lround(pos + error);
    pos += speed;
    error = -sgn(error) * (abs(error) + errorSpeed_);
    const unsigned offset = light * pixelsPerLight_;
    int a = max(0, pos) + offset;
    int b = min(pixelsPerLight_, pos + length) + offset;
    b = max(b, 1);
    if (a >= b) {
        a = b - 1;
    }
    // bigstrobe
    if (doBigStrobe_) {
        int border1 = random(0, pixelCount_);
        int border2 = random(0, pixelCount_);
        a = min(border1, border2);
        b = max(border1, border2);
    }
    // thinning and global distortion
    // Setup discrete probability distribution
    std::vector<int> weights(thinningAmount_, 1);
    std::discrete_distribution<int> discreteDist{weights.begin(), weights.end()};
    for (int i = a; i < b; i++) {
        if (doThinning_) {
            // pick thinning-1 numbers in {0,...,thinning-1}
            std::set<int> choices{};
            for (int j = 0; j < thinningAmount_; j++) {
                choices.insert(discreteDist(randomGenerator_));
            }

            if (choices.find(i % thinningAmount_) != choices.end()) {
                continue;
            }
        }
        if (sampleBernoulli(distortionProb_)) {
            leds[i] = intensityToRgb(0, color);
        } else {
            leds[i] = intensityToRgb(intens, color);
        }
    }
    return showAndMeasureRemainingDuration(1000 / 30);
}

void MovingStrobe::init(unsigned rowCount, unsigned columnCount) {
    AbstractPattern::init(rowCount, columnCount);
    lightCount_ = columnCount_;
    pixelsPerLight_ = rowCount_;
    pixelCount_ = columnCount_ * rowCount_;
    reset();
}
};  // namespace Pattern
