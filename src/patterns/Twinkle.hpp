#pragma once

#include "patterns/AbstractPattern.hpp"

namespace Pattern {
class Twinkle : public AbstractPattern {
   public:
    Twinkle() : AbstractPattern(){};
    unsigned perform(std::vector<CRGB> &leds, CRGB color) override;

   private:
};
};  // namespace Pattern
