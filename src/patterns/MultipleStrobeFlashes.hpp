#pragma once

#include "patterns/AbstractPattern.hpp"

namespace Pattern {
class MultipleStrobeFlashes : public AbstractPattern {
   public:
    MultipleStrobeFlashes() : AbstractPattern(){};
    unsigned perform(std::vector<CRGB> &leds, CRGB color) override;
    void init(unsigned rowCount, unsigned columnCount) override;

   private:
    std::shared_ptr<std::discrete_distribution<int>> probabilityDistribution_{nullptr};
    unsigned minOnDurationMs_{5};
    unsigned maxOnDurationMs_{15};
    unsigned minOffDurationMs_{700};
    unsigned maxOffDurationMs_{8000};
};
};  // namespace Pattern
