#pragma once

#include "patterns/AbstractPattern.hpp"

namespace Pattern {
class RandomSequence : public AbstractPattern {
   public:
    RandomSequence() : AbstractPattern(){};
    unsigned perform(std::vector<CRGB> &leds, CRGB color) override;

   private:
};
};  // namespace Pattern
