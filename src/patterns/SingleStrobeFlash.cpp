#include "patterns/SingleStrobeFlash.hpp"

namespace Pattern {
void SingleStrobeFlash::init(unsigned rowCount, unsigned columnCount) {
    AbstractPattern::init(rowCount, columnCount);
    std::vector<int> distributionWeights = {1, 3, 13, 8, 5, 3};
    probabilityDistribution_ = createDiscreteProbabilityDistribution(distributionWeights);
}

unsigned SingleStrobeFlash::perform(std::vector<CRGB> &leds, CRGB color) {
    std::random_device random_device;
    std::mt19937 random_number_generator(random_device());
    unsigned numOfColsToLightUp = (*probabilityDistribution_)(random_number_generator);
    // Switch up color in 5 percent of cases
    if (random(0, 100) < 5) {
        color = color ^ random(0xffffff + 1);
    }
    auto columnsToLightUp = sampleColumns(numOfColsToLightUp);
    for (unsigned i = 0; i < columnsToLightUp.size(); i++) {
        lightUpColumn(leds, columnsToLightUp[i], color);
    }
    unsigned onDuration = random(minOnDurationMs_, maxOnDurationMs_ + 1);
    delay(onDuration);
    FastLED.clear(true);
    unsigned offDuration = random(minOffDurationMs_, maxOffDurationMs_ + 1);
    return offDuration;
}
};  // namespace Pattern
