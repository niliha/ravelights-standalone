#include "Pattern.hpp"
#include <algorithm>

namespace Pattern {

/* AbstractPattern */
void AbstractPattern::init(unsigned rowCount, unsigned columnCount) {
    rowCount_ = rowCount;
    columnCount_ = columnCount;
}

std::vector<unsigned> AbstractPattern::sampleColumns(unsigned amountOfRowsToSample) {
    // fill remaining columns initially with 0,1,...,amountOfRowsToSample-1
    std::vector<unsigned> remainingColumns(columnCount_);
    std::iota(remainingColumns.begin(), remainingColumns.end(), 0);
    std::vector<unsigned> selectedColumns;
    for (unsigned i = 0; i < amountOfRowsToSample; i++) {
        // select one of the remaining columns
        unsigned selectedColumnIndex = random(0, columnCount_ - i);
        unsigned selectedColumn = remainingColumns[selectedColumnIndex];
        selectedColumns.push_back(selectedColumn);
        remainingColumns.erase(remainingColumns.begin() + selectedColumnIndex);
    }
    return selectedColumns;
}

unsigned AbstractPattern::getStartIndexOfColumn(unsigned column) { return column * rowCount_; }

unsigned AbstractPattern::getEndIndexOfColumn(unsigned column) { return getStartIndexOfColumn(column) + rowCount_ - 1; }

void AbstractPattern::lightUpColumn(std::vector<CRGB> &leds, unsigned columnIndex, CRGB color) {
    for (std::vector<int>::size_type i = getStartIndexOfColumn(columnIndex); i <= getEndIndexOfColumn(columnIndex);
         i++) {
        leds[i] = color;
    }
    FastLED.show();
}

std::shared_ptr<std::discrete_distribution<>>
AbstractPattern::createDiscreteProbabilityDistribution(std::vector<int> &distributionWeights) {
    // Weights which specify the likelihood for each amount of columns {0, ..., columnCount_}
    // that should light up at the same time
    if (distributionWeights.size() > columnCount_ + 1) {
        while (distributionWeights.size() > columnCount_ + 1) {
            distributionWeights.pop_back();
        }
    }
    if (distributionWeights.size() < columnCount_ + 1) {
        while (distributionWeights.size() < columnCount_ + 1) {
            distributionWeights.push_back(1);
        }
    }
    auto probabilityDistribution =
        std::make_shared<std::discrete_distribution<>>(distributionWeights.begin(), distributionWeights.end());
    return probabilityDistribution;
}

unsigned AbstractPattern::invertColor(unsigned color) { return 0xffffff - color; }

bool AbstractPattern::isColumnCompletelyDark(std::vector<CRGB> &leds, unsigned columnIndex) {
    for (unsigned i = getStartIndexOfColumn(columnIndex); i <= getEndIndexOfColumn(columnIndex); i++) {
        if (leds[i] != CRGB(0)) {
            return false;
        }
    }
    return true;
}

unsigned AbstractPattern::flipPixelVertically(unsigned pixelIndex, int pixelColumnIndex, bool flipPixel) {
    if (!flipPixel) {
        return pixelIndex;
    }
    return getEndIndexOfColumn(pixelColumnIndex) - pixelIndex + getStartIndexOfColumn(pixelColumnIndex);
}

/* RandomSequence */
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

/* RandomSegment */
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

        unsigned pixelIntervalLength = random(4, 9);
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

/* SingleStrobeFlash */
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

/* MultipleStrobeFlashes */
void MultipleStrobeFlashes::init(unsigned rowCount, unsigned columnCount) {
    AbstractPattern::init(rowCount, columnCount);
    std::vector<int> distributionWeights = {1, 3, 8, 7, 2, 1};
    probabilityDistribution_ = createDiscreteProbabilityDistribution(distributionWeights);
}

unsigned MultipleStrobeFlashes::perform(std::vector<CRGB> &leds, CRGB color) {
    std::random_device random_device;
    std::mt19937 random_number_generator(random_device());

    unsigned numOfColsToLightUp = (*probabilityDistribution_)(random_number_generator);
    unsigned numOfFlashes = random(1, 15);
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

/* Twinkle */
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
    unsigned offDuration = random(0, 50);
    return offDuration;
}

/* Comet */
void Comet::fadeRandomPixelsToBlackBy(std::vector<CRGB> &leds, unsigned startIndex, unsigned endIndex,
                                      uint8_t fadeAmount) {
    for (unsigned i = startIndex; i <= endIndex; i++) {
        if (random(0, 2) == 1) {
            leds[i].fadeToBlackBy(fadeAmount);
        }
    }
    FastLED.show();
}

unsigned Comet::perform(std::vector<CRGB> &leds, CRGB color) {
    const unsigned cometSize = 20;
    uint8_t fadeAmount = 200;  // Fade to black by 256 / fadeAmount %
    unsigned cometStartIndex = 0;
    unsigned onDuration = 0;
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
                leds[flipPixelVertically(getStartIndexOfColumn(columnIndex) + cometStartIndex + i, columnIndex,
                                         flipPattern)] = color;
            }
        }
        for (auto columnIndex : columnsToLightUp) {
            // Fade half of the LEDs one step
            fadeRandomPixelsToBlackBy(leds, getStartIndexOfColumn(columnIndex), getEndIndexOfColumn(columnIndex),
                                      fadeAmount);
        }
        delay(onDuration);
        cometStartIndex++;
    }
    // Fade remaining pixels to complete darkness
    while (!isColumnCompletelyDark(leds, columnsToLightUp[random(0, columnsToLightUp.size())])) {
        for (auto columnIndex : columnsToLightUp) {
            fadeRandomPixelsToBlackBy(leds, getStartIndexOfColumn(columnIndex), getEndIndexOfColumn(columnIndex),
                                      fadeAmount);
        }
        delay(onDuration);
    }
    unsigned offDuration = random(10, 100);
    return offDuration;
}

unsigned DebugSolidColor::perform(std::vector<CRGB> &leds, CRGB color) {
    FastLED.showColor(color);
    return UINT32_MAX;
}

}  // namespace Pattern
