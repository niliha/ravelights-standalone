#include "Pattern.hpp"
#include <algorithm>
#include <numeric>

namespace Pattern {

std::vector<unsigned> AbstractPattern::sampleColumns(unsigned amountOfRowsToSample) {
  // fill remaining columns with 0,1,...,amount-1
  std::vector<unsigned> remainingColumns(columnCount);
  std::iota(remainingColumns.begin(), remainingColumns.end(), 0); 
  std::vector<unsigned> selectedColumns;
  for (unsigned i = 0; i < amountOfRowsToSample; i++) {
      // select one of the remaining columns
      unsigned selectedColumnIndex = random(0,columnCount-i);
      unsigned selectedColumn = remainingColumns[selectedColumnIndex];
      selectedColumns.push_back(selectedColumn);
      remainingColumns.erase(remainingColumns.begin() + selectedColumnIndex);
  }
  return selectedColumns;
}

unsigned AbstractPattern::getStartIndexOfColumn(unsigned column) { return column * rowCount; }

unsigned AbstractPattern::getEndIndexOfColumn(unsigned column)
{
  return getStartIndexOfColumn(column) + rowCount- 1;
}

void AbstractPattern::lightUpColumn(std::vector<CRGB> &leds, LEDController &ledController, unsigned columnIndex, CRGB color) {
        for (std::vector<int>::size_type i = getStartIndexOfColumn(columnIndex);
             i <= getEndIndexOfColumn(columnIndex) ; i++)
        {
            leds[i] = color;
        }
        ledController.show();
}

unsigned RandomSequence::perform(std::vector<CRGB> &leds,LEDController &ledController, CRGB color)
{
    auto columnsToLightUp = sampleColumns(columnCount);
   for (unsigned i = 0; i < columnsToLightUp.size(); i++) {
       lightUpColumn(leds, ledController, columnsToLightUp[i], color);
        unsigned onDuration = random(30, 60);
        delay(durationOn);
        ledController.clear();
        ledController.show();
   } 
    unsigned offDuration = random(700, 3000);
    return offDuration;
}
} // namespace Pattern