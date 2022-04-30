#pragma once

#include "patterns/AbstractPattern.hpp"

namespace Pattern {
class Blackout : public AbstractPattern {
   public:
    Blackout() : AbstractPattern(){};
    unsigned perform(std::vector<CRGB> &leds, CRGB color) override;

   private:
};
};  // namespace Pattern
