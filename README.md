# RaveLights

A light installation for raves, consisting of multiple tubes with addressable pixel strips, driven by an ESP32 microcontroller.

![Demo video of RaveLights in action](docs/demo.gif)

## Setup

So far, RaveLights has been tested with WS2812 LEDs.
The setup can be seen as one long LED strip with `N * M` LEDs, arranged as matrix with N rows and M columns.

## Getting started

A [PlatformIO installation](https://platformio.org/install) is required, and optionally an IDE integration, e.g. for [VSCode](https://platformio.org/install/ide?install=vscode).  
Using PlatformIO, you can build the project and upload it to an ESP32.

See `src/main.cpp` for usage and adapt the config to your setup.

## Contributing patterns

Patterns are represented by classes that inherit from the abstract base class `AbstractPattern` and implement a method with signature `unsigned perform(std::vector<CRGB> &leds, CRGB color)`, which is called repeatedly by the `RaveLights` instance.
This is where a single cycle of a pattern must be implemented (see existing patterns).
The `leds` vector holds RGB color values for all (`rowCount_` * `columnCount`) pixels which can be modified as desired.
To finally light up the pixels, `FastLED.show()` can be used. See also the convenience methods provided by `AbstractPattern`.
