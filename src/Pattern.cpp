#include "Pattern.hpp"

namespace Pattern {

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

unsigned RandomSegments::perform(std::vector<CRGB> &leds, CRGB color) {
    std::random_device random_device;
    std::mt19937 random_number_generator(random_device());
    unsigned numOfColsToLightUp = probabilityDistribution_(random_number_generator);
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

void RandomSegments::init(unsigned rowCount, unsigned columnCount) {
    AbstractPattern::init(rowCount, columnCount);
    // Weights which specify the likelihood for each amount of columns {0, ..., columnCount_}
    // that should light up at the same time
    std::vector<int> distributionWeights;
    if (columnCount_ >= 5) {
        distributionWeights = {2, 4, 8, 7, 5, 3};
        while (distributionWeights.size() < columnCount_ + 1) {
            distributionWeights.push_back(2);
        }
    } else {
        // uniform distribution
        std::vector<int> distributionWeights(columnCount_ + 1, 1);
    }
    probabilityDistribution_ = std::discrete_distribution<>(distributionWeights.begin(), distributionWeights.end());
}

unsigned SingleStrobeFlash::perform(std::vector<CRGB> &leds, CRGB color) {
    std::random_device random_device;
    std::mt19937 random_number_generator(random_device());
    unsigned numOfColsToLightUp = probabilityDistribution_(random_number_generator);
    // Switch up color in 5 percent of cases
    if (random(0, 100) < 5) {
        color = color ^ random(0xffffff + 1);
    }
    auto columnsToLightUp = sampleColumns(columnCount_);

    for (unsigned i = 0; i < columnsToLightUp.size(); i++) {
        lightUpColumn(leds, columnsToLightUp[i], color);
    }

    unsigned onDuration = random(minOnDurationMs_, maxOnDurationMs_ + 1);
    delay(onDuration);
    FastLED.clear(true);
    unsigned offDuration = random(minOffDurationMs_, maxOffDurationMs_ + 1);
    return offDuration;
}

void SingleStrobeFlash::init(unsigned rowCount, unsigned columnCount) {
    AbstractPattern::init(rowCount, columnCount);
    // Weights which specify the likelihood for each amount of columns {0, ..., columnCount_}
    // that should light up at the same time
    std::vector<int> distributionWeights;
    if (columnCount_ >= 5) {
        distributionWeights = {1, 3, 5, 4, 6, 10};
        while (distributionWeights.size() < columnCount_ + 1) {
            distributionWeights.push_back(2);
        }
    } else {
        // uniform distribution
        std::vector<int> distributionWeights(columnCount_ + 1, 1);
    }
    probabilityDistribution_ = std::discrete_distribution<>(distributionWeights.begin(), distributionWeights.end());
}

}  // namespace Pattern
