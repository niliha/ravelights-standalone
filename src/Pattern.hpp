#pragma once
#include "FastLED.h"
#include <vector>

namespace Pattern
{

    class AbstractPattern
    {
    public:
        AbstractPattern(const unsigned rowCount, const unsigned columnCount) : rowCount(rowCount), columnCount(columnCount){};
        virtual unsigned perform(std::vector<CRGB> &leds, CRGB color) = 0;

    protected:
        const unsigned rowCount;
        const unsigned columnCount;
        // Utility functions used across patterns here...
        //
        std::vector<unsigned> sampleColumns(unsigned columnCount);
        
    unsigned getStartIndexOfColumn(unsigned column);

    unsigned getEndIndexOfColumn(unsigned column);

void lightUpColumn(unsigned columnIndex, CRGB color);

    private:
    };

    class RandomSequence : public AbstractPattern
    {
    public:
        RandomSequence(const unsigned rowCount, const unsigned columnCount) : AbstractPattern(rowCount, columnCount){};
        unsigned perform(std::vector<CRGB> &leds,LEDController &ledController, CRGB color;
         ) override;

    private:
    };
}