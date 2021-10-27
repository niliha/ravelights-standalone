#include "Pattern.hpp"

namespace Pattern
{

  void AbstractPattern::init(unsigned rowCount, unsigned columnCount)
  {
    rowCount_ = rowCount;
    columnCount_ = columnCount;
  }

  std::vector<unsigned> AbstractPattern::sampleColumns(unsigned amountOfRowsToSample)
  {
    // fill remaining columns initially with 0,1,...,amountOfRowsToSample-1
    std::vector<unsigned> remainingColumns(columnCount_);
    std::iota(remainingColumns.begin(), remainingColumns.end(), 0);
    std::vector<unsigned> selectedColumns;
    for (unsigned i = 0; i < amountOfRowsToSample; i++)
    {
      // select one of the remaining columns
      unsigned selectedColumnIndex = random(0, columnCount_ - i);
      unsigned selectedColumn = remainingColumns[selectedColumnIndex];
      selectedColumns.push_back(selectedColumn);
      remainingColumns.erase(remainingColumns.begin() + selectedColumnIndex);
    }
    return selectedColumns;
  }

  unsigned AbstractPattern::getStartIndexOfColumn(unsigned column) { return column * rowCount_; }

  unsigned AbstractPattern::getEndIndexOfColumn(unsigned column)
  {
    return getStartIndexOfColumn(column) + rowCount_ - 1;
  }

  void AbstractPattern::lightUpColumn(std::vector<CRGB> &leds, unsigned columnIndex, CRGB color)
  {
    for (std::vector<int>::size_type i = getStartIndexOfColumn(columnIndex);
         i <= getEndIndexOfColumn(columnIndex); i++)
    {
      leds[i] = color;
    }
    FastLED.show();
  }

  unsigned RandomSequence::perform(std::vector<CRGB> &leds, CRGB color)
  {
    auto columnsToLightUp = sampleColumns(columnCount_);
    for (unsigned i = 0; i < columnsToLightUp.size(); i++)
    {
      lightUpColumn(leds, columnsToLightUp[i], color);
      unsigned onDuration = random(30, 60);
      delay(onDuration);
      FastLED.clear(true);
    }
    unsigned offDuration = random(700, 3000);
    return offDuration;
  }
} // namespace Pattern