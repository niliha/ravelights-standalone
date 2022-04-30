#include "patterns/RandomSegments.hpp"

namespace Pattern {
void RandomSegments::init(unsigned rowCount, unsigned columnCount) {
    AbstractPattern::init(rowCount, columnCount);
    std::vector<int> distributionWeights{1, 4, 8, 7, 5, 5};
    probabilityDistribution_ = createDiscreteProbabilityDistribution(distributionWeights);
}

unsigned RandomSegments::perform(std::vector<CRGB> &leds, CRGB color) {
    std::random_device random_device;
    std::mt19937 random_number_generator(random_device());
    unsigned numOfColsToLightUp = (*probabilityDistribution_)(random_number_generator);
    // Switch up color in 10 percent of cases
    if (random(0, 100) < 10) {
        color = color ^ random(0xffffff + 1);
    }
    for (unsigned i = 0; i < numOfColsToLightUp; i++) {
        unsigned columnToLightUp = random(columnCount_);

        unsigned pixelIntervalLength = random(rowCount_ / 8, rowCount_ / 4);
        unsigned pixelIntervalStart =
            getStartIndexOfColumn(columnToLightUp) + random(0, rowCount_ - pixelIntervalLength);
        unsigned pixelIntervalEnd = pixelIntervalStart + pixelIntervalLength;
        for (unsigned j = pixelIntervalStart; j < pixelIntervalEnd + 1; j++) {
            leds[j] = color;
        }
    }
    FastLED.show();
    unsigned onDuration = random(minOnDurationMs_, maxOnDurationMs_ + 1);
    delay(onDuration);
    FastLED.clear(true);
    unsigned offDuration = random(minOffDurationMs_, maxOffDurationMs_ + 1);
    return offDuration;
}
};  // namespace Pattern
