#include "Pattern.hpp"
#include <algorithm>
#include <cmath>
#include <set>

namespace Pattern {

template <typename T> int sgn(T val) { return (T(0) < val) - (val < T(0)); }

/* AbstractPattern */
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
    unsigned offDuration = random(0, 10);
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

/* MovingStrobe */
MovingStrobe::MovingStrobe(double p_bigstrobe, double p_pause, double p_thin)
    : AbstractPattern(), bigStrobeProb_(p_bigstrobe), pauseProb_(p_pause), thinningProb_(p_thin) {
    reset();
}

void MovingStrobe::reset() {
    distortionProb_ = uniformDist_005_02_(randomGenerator_);
    light = random(lightCount_);
    frame = 0;
    pos = max(std::lround(abs(normalDist_0_1_(randomGenerator_) * pixelCount_)), (long)0);
    error = 0;
    speed = random(1, 5 + 1);
    length = random(5, 30 + 1);
    maxFrameCount_ = random(5, 25 + 1);
    errorSpeed_ = max(normalDist_2_05_(randomGenerator_), (double)1);

    // special mode : bigstrobe
    doBigStrobe_ = false;
    if (sampleBernoulli(bigStrobeProb_)) {
        doBigStrobe_ = true;
        maxFrameCount_ = random(2, 10);
    }
    // special mode : thinned LED
    doThinning_ = false;
    thinningAmount_ = 10;
    if (sampleBernoulli(thinningProb_)) {
        doThinning_ = true;
    }
    // special mode : pause
    doPause_ = false;
    if (sampleBernoulli(pauseProb_)) {
        doPause_ = true;
    }
}

unsigned MovingStrobe::perform(std::vector<CRGB> &leds, CRGB color) {
    FastLED.clearData();
    frame++;
    // see if animation is finished
    if (frame > maxFrameCount_) {
        reset();
    }
    if (doPause_) {
        return showAndMeasureRemainingDuration(1000 / 30);
    }
    // apply direction
    // !possibly broken
    int direction = 1;
    if (doRandomDirection_ && !sampleBernoulli(0.5)) {
        direction = -1;
    }
    // get intensity
    double intens = min((double)1, abs(std::sin(frame * sinFactor_)) + 0.1);
    // update position
    pos = direction * std::lround(pos + error);
    pos += speed;
    error = -sgn(error) * (abs(error) + errorSpeed_);
    const unsigned offset = light * pixelsPerLight_;
    int a = max(0, pos) + offset;
    int b = min(pixelsPerLight_, pos + length) + offset;
    b = max(b, 1);
    if (a >= b) {
        a = b - 1;
    }
    // bigstrobe
    if (doBigStrobe_) {
        int border1 = random(0, pixelCount_);
        int border2 = random(0, pixelCount_);
        a = min(border1, border2);
        b = max(border1, border2);
    }
    // thinning and global distortion
    //  Setup discrete probability distribution
    std::vector<int> weights(thinningAmount_, 1);
    std::discrete_distribution<int> discreteDist{weights.begin(), weights.end()};
    for (int i = a; i < b; i++) {
        if (doThinning_) {
            // pick thinning-1 numbers in {0,...,thinning-1}
            std::set<int> choices{};
            for (int j = 0; j < thinningAmount_; j++) {
                choices.insert(discreteDist(randomGenerator_));
            }

            if (choices.find(i % thinningAmount_) != choices.end()) {
                continue;
            }
        }
        if (sampleBernoulli(distortionProb_)) {
            leds[i] = intensityToRgb(0, color);
        } else {
            leds[i] = intensityToRgb(intens, color);
        }
    }
    return showAndMeasureRemainingDuration(1000 / 30);
}

void MovingStrobe::init(unsigned rowCount, unsigned columnCount) {
    AbstractPattern::init(rowCount, columnCount);
    lightCount_ = columnCount_;
    pixelsPerLight_ = rowCount_;
    pixelCount_ = columnCount_ * rowCount_;
    reset();
}

}  // namespace Pattern

uint16_t XY(uint8_t x, uint8_t y) { return x * y + y; }
