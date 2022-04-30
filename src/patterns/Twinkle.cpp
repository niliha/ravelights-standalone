#include "patterns/Twinkle.hpp"

namespace Pattern {
unsigned Twinkle::perform(std::vector<CRGB> &leds, CRGB color) {

    unsigned ledCount = rowCount_ * columnCount_;
    unsigned spotCount = random(5, 50);
    for (unsigned i = 0; i < spotCount; i++) {
        unsigned pixelIndex = random(ledCount);
        // Always light up chosen pixel
        leds[pixelIndex] = color;
        // Light up neighboring pixels with 50% probability each
        if (sampleBernoulli(0.5)) {
            leds[(pixelIndex - 1) % (ledCount)] = color;
        }
        if (sampleBernoulli(0.5)) {
            leds[(pixelIndex + 1) % ledCount] = color;
        }
    }
    FastLED.show();
    unsigned offDuration = random(0, 2);
    return offDuration;
}
};  // namespace Pattern
