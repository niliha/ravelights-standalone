#include "patterns/RandomSequence.hpp"

namespace Pattern {
unsigned RandomSequence::perform(std::vector<CRGB> &leds, CRGB color) {
    auto columnsToLightUp = sampleColumns(columnCount_);
    for (unsigned i = 0; i < columnsToLightUp.size(); i++) {
        lightUpColumn(leds, columnsToLightUp[i], color);
        unsigned onDuration = random(30, 60);
        delay(onDuration);
        FastLED.clear(true);
    }
    unsigned offDuration = random(700, 3000);
    return offDuration;
}
};  // namespace Pattern
