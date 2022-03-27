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

class DebugSolidColor : public AbstractPattern {
   public:
    DebugSolidColor() : AbstractPattern(){};
    unsigned perform(std::vector<CRGB> &leds, CRGB color) override;

   private:
};

class DebugStrobe : public AbstractPattern {
   public:
    DebugStrobe() : AbstractPattern(){};
    unsigned perform(std::vector<CRGB> &leds, CRGB color) override;

   private:
};

class Explosion : public AbstractPattern {
   public:
    Explosion() : AbstractPattern(){};
    unsigned perform(std::vector<CRGB> &leds, CRGB color) override;

   private:
};

class MovingStrobe : public AbstractPattern {
   public:
    MovingStrobe();  // : AbstractPattern(), n_lights(columnCount_), n_leds(rowCount_), n(columnCount_ * rowCount_){};

    unsigned perform(std::vector<CRGB> &leds, CRGB color) override;
    void init(unsigned rowCount, unsigned columnCount) override;

   private:
    const std::array<double, 2> distort_chance_param{{0.05, 0.2}};
    const double sin_factor = 2;

    unsigned n_lights;

    unsigned n_leds;
    unsigned n;
    double distort_chance;
    std::random_device random_device;  // Will be used to obtain a seed for the random number engine
    std::mt19937 generator;            // Standard mersenne_twister_engine seeded with rd()
    std::uniform_real_distribution<> uniform_dist_005_02{0.05, 0.2};
    std::uniform_real_distribution<> uniform_dist_0_1{0, 1};
    std::normal_distribution<> normal_dist_0_1{0, 1};
    std::normal_distribution<> normal_dist_2_05{2, 0.5};
    unsigned frame;
    int pos;
    unsigned light;
    int error;
    unsigned speed;
    unsigned length;
    unsigned max_frame;
    double error_speed;
    bool mode_bigstrobe;
    bool mode_pause;
    bool mode_thinned;
    unsigned thinning;
    double p_bigstrobe;
    double p_pause;
    double p_thin;
    bool random_direction;

    void reset();
    bool p(double chance);
    CRGB intensityToRgb(double intensity, CRGB color);
};

}  // namespace Pattern
