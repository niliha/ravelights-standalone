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
    unsigned minOnDurationMs_{20};
    unsigned maxOnDurationMs_{20};
    unsigned minOffDurationMs_{700};
    unsigned maxOffDurationMs_{5000};
};
};  // namespace Pattern

