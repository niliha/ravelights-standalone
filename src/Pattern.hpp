#pragma once
#include "FastLED.h"
#include <memory>
#include <random>
#include <vector>

namespace Pattern {

class AbstractPattern {
   public:
    AbstractPattern(){};
    virtual void init(unsigned rowCount, unsigned columnCount);
    virtual unsigned perform(std::vector<CRGB> &leds, CRGB color) = 0;

   protected:
    unsigned rowCount_{0};
    unsigned columnCount_{0};
    std::default_random_engine randomGenerator_;

    // Utility functions used across patterns
    std::vector<unsigned> sampleColumns(unsigned columnCount);
    unsigned getStartIndexOfColumn(unsigned column);
    unsigned getEndIndexOfColumn(unsigned column);
    void lightUpColumn(std::vector<CRGB> &leds, unsigned columnIndex, CRGB color, bool writeLeds = true);
    std::shared_ptr<std::discrete_distribution<>>
    createDiscreteProbabilityDistribution(std::vector<int> &distributionWeights);
    unsigned invertColor(unsigned color);
    bool isColumnCompletelyDark(std::vector<CRGB> &leds, unsigned columnIndex);
    unsigned flipPixelVertically(unsigned pixelIndex, int pixelColumnIndex, bool flipPixel = true);
    unsigned showAndMeasureRemainingDuration(unsigned delayMs);
    void showForEffectiveDuration(unsigned delayMs);
    void indexToCoordinates(unsigned pixelIndex, unsigned &columnIndex, unsigned &rowIndex);
    unsigned coordinatesToIndex(unsigned columnIndex, unsigned rowIndex);
    bool sampleBernoulli(double chance);
    CRGB intensityToRgb(double intensity, CRGB color);

   private:
};

class RandomSequence : public AbstractPattern {
   public:
    RandomSequence() : AbstractPattern(){};
    unsigned perform(std::vector<CRGB> &leds, CRGB color) override;

   private:
};

class RandomSegments : public AbstractPattern {
   public:
    RandomSegments() : AbstractPattern(){};
    unsigned perform(std::vector<CRGB> &leds, CRGB color) override;
    void init(unsigned rowCount, unsigned columnCount) override;

   private:
    std::shared_ptr<std::discrete_distribution<int>> probabilityDistribution_;
    unsigned minOnDurationMs_{100};
    unsigned maxOnDurationMs_{500};
    unsigned minOffDurationMs_{300};
    unsigned maxOffDurationMs_{1000};
};

class SingleStrobeFlash : public AbstractPattern {
   public:
    SingleStrobeFlash() : AbstractPattern(){};
    unsigned perform(std::vector<CRGB> &leds, CRGB color) override;
    void init(unsigned rowCount, unsigned columnCount) override;

   private:
    std::shared_ptr<std::discrete_distribution<int>> probabilityDistribution_;
    unsigned minOnDurationMs_{50};
    unsigned maxOnDurationMs_{200};
    unsigned minOffDurationMs_{1000};
    unsigned maxOffDurationMs_{8000};
};

class MultipleStrobeFlashes : public AbstractPattern {
   public:
    MultipleStrobeFlashes() : AbstractPattern(){};
    unsigned perform(std::vector<CRGB> &leds, CRGB color) override;
    void init(unsigned rowCount, unsigned columnCount) override;

   private:
    std::shared_ptr<std::discrete_distribution<int>> probabilityDistribution_{nullptr};
    unsigned minOnDurationMs_{20};
    unsigned maxOnDurationMs_{20};
    unsigned minOffDurationMs_{700};
    unsigned maxOffDurationMs_{5000};
};

class Twinkle : public AbstractPattern {
   public:
    Twinkle() : AbstractPattern(){};
    unsigned perform(std::vector<CRGB> &leds, CRGB color) override;

   private:
};

class Comet : public AbstractPattern {
   public:
    Comet() : AbstractPattern(){};
    unsigned perform(std::vector<CRGB> &leds, CRGB color) override;

   private:
    void fadeRandomPixelsToBlackBy(std::vector<CRGB> &leds, unsigned startIndex, unsigned endIndex, uint8_t fadeAmount);
};

class MovingStrobe : public AbstractPattern {
   public:
    MovingStrobe(double p_bigstrobe = 0.3, double p_pause = 0.5,
                 double p_thin = 0.4);  // : AbstractPattern(), n_lights(columnCount_), n_leds(rowCount_),
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

}  // namespace Pattern
