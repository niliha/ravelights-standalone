#pragma once
#include "FastLED.h"
#include <vector>

namespace Pattern
{

    class AbstractPattern
    {
    public:
        AbstractPattern(){};
        void init(unsigned rowCount, unsigned columnCount);
        virtual unsigned perform(std::vector<CRGB> &leds, CRGB color) = 0;

    protected:
        unsigned rowCount_ = 0;
        unsigned columnCount_ = 0;
        // Utility functions used across patterns
        std::vector<unsigned> sampleColumns(unsigned columnCount);

        unsigned getStartIndexOfColumn(unsigned column);

        unsigned getEndIndexOfColumn(unsigned column);

        void lightUpColumn(std::vector<CRGB> &leds, unsigned columnIndex, CRGB color);

    private:
    };

    class RandomSequence : public AbstractPattern
    {
    public:
        RandomSequence() : AbstractPattern(){};
        unsigned perform(std::vector<CRGB> &leds, CRGB color) override;

    private:
    };
}