#include "patterns/Twinkle.hpp"

namespace Pattern {
unsigned Twinkle::perform(std::vector<CRGB> &leds, CRGB color) {
    unsigned ledCount = rowCount_ * columnCount_;
    unsigned pixelIndex = random(ledCount);
    // Always light up chosen pixel
    leds[pixelIndex] = color;
    // Light up neighboring pixels with 50% probability each
    if (random(0, 2) == 1) {
        leds[(pixelIndex - 1) % (ledCount)] = color;
    }
    if (random(0, 2) == 1) {
        leds[(pixelIndex + 1) % ledCount] = color;
    }
    FastLED.show();
    unsigned offDuration = random(0, 10);
    return offDuration;
}
};  // namespace Pattern
