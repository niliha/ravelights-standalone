#include "patterns/MultipleStrobeFlashes.hpp"

namespace Pattern {
void MultipleStrobeFlashes::init(unsigned rowCount, unsigned columnCount) {
    AbstractPattern::init(rowCount, columnCount);
    std::vector<int> distributionWeights = {1, 3, 8, 7, 2, 1};
    probabilityDistribution_ = createDiscreteProbabilityDistribution(distributionWeights);
}

unsigned MultipleStrobeFlashes::perform(std::vector<CRGB> &leds, CRGB color) {
    std::random_device random_device;
    std::mt19937 random_number_generator(random_device());

    unsigned numOfColsToLightUp = (*probabilityDistribution_)(random_number_generator);
    unsigned numOfFlashes = random(1, 20);
    unsigned columnIndexWithInvertedColor = random(0, columnCount_);
    for (unsigned i = 0; i < numOfFlashes; i++) {
        auto columnsToLightUp = sampleColumns(numOfColsToLightUp);
        for (unsigned columnIndex = 0; columnIndex < columnsToLightUp.size(); columnIndex++) {
            auto colorToShow = color;
            if (columnIndex == columnIndexWithInvertedColor) {
                colorToShow = invertColor(color);
            }
            lightUpColumn(leds, columnsToLightUp[columnIndex], colorToShow);
        }
        FastLED.show();
        unsigned onDuration = random(minOnDurationMs_, maxOnDurationMs_ + 1);
        delay(onDuration);
        FastLED.clear(true);
        delay(onDuration);
    }
    FastLED.clear(true);
    unsigned offDuration = random(minOffDurationMs_, maxOffDurationMs_ + 1);
    return offDuration;
}
};  // namespace Pattern
