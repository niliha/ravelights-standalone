#pragma once

#include "patterns/AbstractPattern.hpp"

namespace Pattern {
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
};  // namespace Pattern
