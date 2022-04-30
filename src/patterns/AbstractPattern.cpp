#include "patterns/AbstractPattern.hpp"

namespace Pattern {
void AbstractPattern::init(unsigned rowCount, unsigned columnCount) {
    rowCount_ = rowCount;
    columnCount_ = columnCount;
    // esp_random() provides true random value if either WIFI or bluetooth is running
    randomGenerator_.seed(esp_random());
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

void AbstractPattern::lightUpColumn(std::vector<CRGB> &leds, unsigned columnIndex, CRGB color, bool writeLeds) {
    for (std::vector<int>::size_type i = getStartIndexOfColumn(columnIndex); i <= getEndIndexOfColumn(columnIndex);
         i++) {
        leds[i] = color;
    }
    if (writeLeds) {
        FastLED.show();
    }
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
void AbstractPattern::showForEffectiveDuration(unsigned delayMs) { delay(showAndMeasureRemainingDuration(delayMs)); }

unsigned AbstractPattern::showAndMeasureRemainingDuration(unsigned delayMs) {
    unsigned long timeBeforeShow = millis();
    FastLED.show();
    unsigned passedTimeMs = millis() - timeBeforeShow;
    if (delayMs > passedTimeMs) {
        return delayMs - passedTimeMs;
    }
    return 0;
}
void AbstractPattern::indexToCoordinates(unsigned pixelIndex, unsigned &columnIndex, unsigned &rowIndex) {
    columnIndex = pixelIndex / columnCount_;
    rowIndex = pixelIndex % columnCount_;
}
unsigned AbstractPattern::coordinatesToIndex(unsigned columnIndex, unsigned rowIndex) {
    return columnIndex * rowCount_ + rowIndex;
}

bool AbstractPattern::sampleBernoulli(double probability) {
    std::bernoulli_distribution bernoulliDistr(probability);
    return bernoulliDistr(randomGenerator_);
}

CRGB AbstractPattern::intensityToRgb(double intensity, CRGB color) {
    color.red = ((double)color.red) * intensity;
    color.green = ((double)color.green) * intensity;
    color.blue = ((double)color.blue) * intensity;
    return color;
}

};  // namespace Pattern
