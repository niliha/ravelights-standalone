#pragma once

#include "patterns/AbstractPattern.hpp"

namespace Pattern {
class Comet : public AbstractPattern {
   public:
    Comet() : AbstractPattern(){};
    unsigned perform(std::vector<CRGB> &leds, CRGB color) override;

   private:
    void fadeRandomPixelsToBlackBy(std::vector<CRGB> &leds, unsigned startIndex, unsigned endIndex, uint8_t fadeAmount);
};
};  // namespace Pattern
