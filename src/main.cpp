#include "Network.hpp"
#include "Pattern.hpp"
#include "RaveLights.hpp"
#include "WifiCredentials.hpp"

#include <Arduino.h>
#include <FastLED.h>

// Must be known at compile time for FastLED
const unsigned LED_ROW_COUNT = 30;
const unsigned LED_COLUMN_COUNT = 5;

// Vector of shared_ptr's to Pattern Instances that will be added to the RaveLights instance
std::vector<std::shared_ptr<Pattern::AbstractPattern>> patterns{
    std::make_shared<Pattern::RandomSegments>(),    // 0
    std::make_shared<Pattern::RandomSequence>(),    // 1
    std::make_shared<Pattern::SingleStrobeFlash>()  // 2
};

// RaveLights instance
RaveLights<LED_ROW_COUNT, LED_COLUMN_COUNT> raveLights;

void setup() {
    Serial.begin(115200);
    randomSeed(analogRead(2));
    // Network::initWifiAccessPoint(WifiCredentials::ssid, WifiCredentials::password);
    Network::connectToWifi(WifiCredentials::ssid, WifiCredentials::password);

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
