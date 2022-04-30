#pragma once

#include "patterns/AbstractPattern.hpp"

namespace Pattern {
class MovingStrobe : public AbstractPattern {
   public:
    MovingStrobe(double p_bigstrobe = 0.3, double p_pause = 0.5,
                 double p_thin = 0.1);  // : AbstractPattern(), n_lights(columnCount_), n_leds(rowCount_),
                                        // n(columnCount_ * rowCount_){};

    unsigned perform(std::vector<CRGB> &leds, CRGB color) override;
    void init(unsigned rowCount, unsigned columnCount) override;

   private:
    unsigned lightCount_;
    unsigned pixelsPerLight_;
    unsigned pixelCount_;

    const double bigStrobeProb_;
    const double pauseProb_;
    const double thinningProb_;
    double distortionProb_;
    const bool doRandomDirection_{true};

    const std::array<double, 2> distortChanceParam_{{0.05, 0.2}};
    const double sinFactor_ = 2;

    std::uniform_real_distribution<> uniformDist_005_02_{0.05, 0.2};
    std::uniform_real_distribution<> uniformDist_0_1_{0, 1};
    std::normal_distribution<> normalDist_0_1_{0, 1};
    std::normal_distribution<> normalDist_2_05_{2, 0.5};

    unsigned frame;
    int pos;
    unsigned light;
    int error;
    unsigned speed;
    unsigned length;
    unsigned maxFrameCount_;
    double errorSpeed_;
    bool doBigStrobe_;
    bool doPause_;
    bool doThinning_;
    unsigned thinningAmount_;

    void reset();
};
};  // namespace Pattern
