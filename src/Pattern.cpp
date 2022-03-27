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

/* DebugSolidColor */
unsigned DebugSolidColor::perform(std::vector<CRGB> &leds, CRGB color) {
    FastLED.showColor(color);
    return UINT32_MAX;
}
/* DebugStrobe */
unsigned DebugStrobe::perform(std::vector<CRGB> &leds, CRGB color) {
    unsigned onDuration = 20;
    lightUpColumn(leds, 0, color, false);
    showForEffectiveDuration(onDuration);
    FastLED.clear(true);
    // return showAndMeasureRemainingDuration(onDuration);
    return 1000;
}

unsigned Explosion::perform(std::vector<CRGB> &leds, CRGB color) {}

/* MovingStrobe */
MovingStrobe::MovingStrobe()
    : AbstractPattern(), n_lights(columnCount_), n_leds(rowCount_), n(columnCount_ * rowCount_),
      generator(random_device()) {

    reset();
}

void MovingStrobe::reset() {
    distort_chance = uniform_dist_005_02(generator);
    light = random(n_lights);
    frame = 0;
    pos = max(std::lround(abs(normal_dist_0_1(generator) * n)), (long)0);
    error = 0;
    speed = random(1, 5 + 1);
    length = random(5, 30 + 1);
    max_frame = random(5, 40 + 1);
    error_speed = max(normal_dist_2_05(generator), (double)1);
    p_bigstrobe = 0.2;
    p_pause = 0.5;
    p_thin = 0.1;
    random_direction = true;

    mode_bigstrobe = true;
    mode_pause = false;
    mode_thinned = false;

    // special mode : bigstrobe
    if (p(p_bigstrobe)) {
        mode_bigstrobe = true;
        max_frame = random(2, 10);
    }
    // special mode : thinned LED
    thinning = 10;
    if (p(p_thin)) {
        mode_thinned = true;
    }
    // special mode : pause
    if (p(p_pause)) {
        mode_pause = true;
    }
}

unsigned MovingStrobe::perform(std::vector<CRGB> &leds, CRGB color) {
    FastLED.clearData();
    frame++;
    // see if animation is finished
    if (frame > max_frame) {
        reset();
    }
    if (mode_pause) {
        Serial.println("Returning because mode_pause=true");
        return showAndMeasureRemainingDuration(1000 / 30);
        //    FastLED.show();
        //   return (30);
    }
    // #apply direction
    // #!possibly broken
    int direction = 1;
    if (random_direction && !p(0.5)) {
        direction = -1;
    }
    // #get intensity
    double intens = min((double)1, abs(std::sin(frame * sin_factor)) + 0.1);
    // #update position
    pos = direction * std::lround(pos + error);
    pos += speed;
    error = -sgn(error) * (abs(error) + error_speed);
    //#apply pixel range
    Serial.print("pos: ");
    Serial.println(pos);
    Serial.print("light: ");
    Serial.println(light);
    Serial.print("n_leds: ");
    Serial.println(n_leds);
    Serial.print("n ");
    Serial.println(n);
    unsigned offset = light * n_leds;
    Serial.print("Offset: ");
    Serial.println(offset);
    int a = max(0, pos) + offset;
    int b = min(n_leds, pos + length) + offset;
    b = max(b, 1);
    if (a >= b) {
        a = b - 1;
    }
    // #bigstrobe
    if (mode_bigstrobe) {
        int border1 = random(0, n);
        int border2 = random(0, n);
        a = min(border1, border2);
        b = max(border1, border2);
    }
    //#global distortion
    // Setup discrete probability distribution
    std::vector<int> weights(thinning, 1);
    std::discrete_distribution<int> discrete_dist{weights.begin(), weights.end()};
    Serial.print("Entering loop from ");
    Serial.print(a);
    Serial.print(" to ");
    Serial.println(b);
    for (int i = a; i < b; i++) {
        if (mode_thinned) {
            // pick thinning-1 numbers in {0,...,thinning-1}
            std::set<int> choices{};
            for (int j = 0; j < thinning; j++) {
                choices.insert(discrete_dist(generator));
            }

            if (choices.find(i % thinning) != choices.end()) {
                continue;
            }
        }
        if (p(distort_chance)) {
            leds[i] = intensityToRgb(0, CRGB::Red);
        } else {
            leds[i] = intensityToRgb(intens, CRGB::Red);
        }
    }

    Serial.println("Returning at end of perform()");
    // FastLED.show();
    return showAndMeasureRemainingDuration(1000 / 30);
}

bool MovingStrobe::p(double chance) { return uniform_dist_0_1(generator) < chance; }

CRGB MovingStrobe::intensityToRgb(double intensity, CRGB color) {

    int discrete_intensity = ((double)255) * intensity;
    color.red = ((double)color.red) * intensity;
    color.green = ((double)color.green) * intensity;
    color.blue = ((double)color.blue) * intensity;
    // return scaled_color;
    return color;
}

void MovingStrobe::init(unsigned rowCount, unsigned columnCount) {
    AbstractPattern::init(rowCount, columnCount);

    n_lights = columnCount_;
    n_leds = rowCount_;
    n = columnCount_ * rowCount_;
    reset();
}

}  // namespace Pattern
