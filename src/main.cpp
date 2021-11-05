#include "Network.hpp"
#include "Pattern.hpp"
#include "RaveLights.hpp"

#include <Arduino.h>
#include <FastLED.h>

// Must be known at compile time for FastLED
const unsigned LED_ROW_COUNT = 30;
const unsigned LED_COLUMN_COUNT = 5;

// Vector of shared_ptr's to Pattern Instances that will be added to the RaveLights instance
std::vector<std::shared_ptr<Pattern::AbstractPattern>> patterns{std::make_shared<Pattern::RandomSegments>(),
                                                                std::make_shared<Pattern::RandomSequence>(),
                                                                std::make_shared<Pattern::SingleStrobeFlash>()};

// RaveLights instance
RaveLights<LED_ROW_COUNT, LED_COLUMN_COUNT> raveLights;

void setup() {
    Serial.begin(115200);
    Network::initWifiAccessPoint("esp32", "ravelights");

    for (auto &pattern : patterns) {
        raveLights.addPattern(pattern);
    }
    Serial.println("Testing LEDs...");
    raveLights.testLeds();
    Serial.println("Starting web server...");
    raveLights.startWebServer();
    Serial.println("Starting show loop...");
    raveLights.startShowLoop();  // does not return at the moment
}

void loop() {}
