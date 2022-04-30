#include "patterns/Comet.hpp"

namespace Pattern {
void Comet::fadeRandomPixelsToBlackBy(std::vector<CRGB> &leds, unsigned startIndex, unsigned endIndex,
                                      uint8_t fadeAmount) {
    for (unsigned i = startIndex; i <= endIndex; i++) {
        if (random(0, 2) == 1) {
            leds[i].fadeToBlackBy(fadeAmount);
        }
    }
}

unsigned Comet::perform(std::vector<CRGB> &leds, CRGB color) {
    const unsigned cometSize = rowCount_ / 8;
    uint8_t fadeAmount = 100;  // Decrease brightness by (fadeAmount/ 256) * brightness
    unsigned cometStartIndex = 0;
    unsigned onDuration = 10;
    bool flipPattern = false;
    if (random(0, 2) == 1) {
        flipPattern = true;
    }
    unsigned numOfColumnsToLightup = 0;
    if (columnCount_ >= 5) {
        numOfColumnsToLightup = random(2, 4);
    } else {
        numOfColumnsToLightup = random(1, columnCount_ + 1);
    }
    auto columnsToLightUp = sampleColumns(numOfColumnsToLightup);
    // Draw comet and its trail until the end of the column is reached
    while (cometStartIndex + cometSize < rowCount_) {
        // Draw comet
        for (unsigned i = 0; i < cometSize; i++) {
            for (auto columnIndex : columnsToLightUp) {
                for (unsigned j = 0; j < columnCount_; j++) {
                    // Move comet columnCount_ pixels at once to increase speed
                    leds[flipPixelVertically(getStartIndexOfColumn(columnIndex) + cometStartIndex + i + j, columnIndex,
                                             flipPattern)] = color;
                }
            }
        }
        for (auto columnIndex : columnsToLightUp) {
            // Fade half of the LEDs one step
            fadeRandomPixelsToBlackBy(leds, getStartIndexOfColumn(columnIndex), getEndIndexOfColumn(columnIndex),
                                      fadeAmount);
        }
        showForEffectiveDuration(onDuration);
        cometStartIndex += 1 + columnCount_;
    }

    // Fade remaining pixels to complete darkness
    while (!isColumnCompletelyDark(leds, columnsToLightUp[random(0, columnsToLightUp.size())])) {
        for (auto columnIndex : columnsToLightUp) {
            fadeRandomPixelsToBlackBy(leds, getStartIndexOfColumn(columnIndex), getEndIndexOfColumn(columnIndex),
                                      fadeAmount);
        }
        showForEffectiveDuration(onDuration);
    }

    unsigned offDuration = random(10, 100);
    return offDuration;
}
};  // namespace Pattern
