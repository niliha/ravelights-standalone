#pragma once

#include "patterns/AbstractPattern.hpp"

namespace Pattern {
class SingleStrobeFlash : public AbstractPattern {
   public:
    SingleStrobeFlash() : AbstractPattern(){};
    unsigned perform(std::vector<CRGB> &leds, CRGB color) override;
    void init(unsigned rowCount, unsigned columnCount) override;

   private:
    std::shared_ptr<std::discrete_distribution<int>> probabilityDistribution_;
    unsigned minOnDurationMs_{10};
    unsigned maxOnDurationMs_{200};
    unsigned minOffDurationMs_{1000};
    unsigned maxOffDurationMs_{8000};
};
};  // namespace Pattern
