#include "patterns/Blackout.hpp"

namespace Pattern {

unsigned Blackout::perform(std::vector<CRGB> &leds, CRGB color) {
    FastLED.clear(true);
    return 0;
}
};  // namespace Pattern
