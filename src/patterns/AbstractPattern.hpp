#pragma once

#define FASTLED_ESP32_I2S  // Alternative parallel output driver
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

// template <typename T> int sgn(T val);
};  // namespace Pattern
