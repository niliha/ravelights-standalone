#pragma once
#include "FastLED.h"
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

    void lightUpColumn(std::vector<CRGB> &leds, unsigned columnIndex, CRGB color);

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
    std::discrete_distribution<int> probabilityDistribution_;
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
    std::discrete_distribution<int> probabilityDistribution_;
    unsigned minOnDurationMs_{50};
    unsigned maxOnDurationMs_{200};
    unsigned minOffDurationMs_{1000};
    unsigned maxOffDurationMs_{8000};
};
}  // namespace Pattern
