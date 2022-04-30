#include "Network.hpp"
#include "Pattern.hpp"
#include "RaveLights.hpp"
#include "WifiCredentials.hpp"
#include <esp_pthread.h>

#include <Arduino.h>
#include <FastLED.h>

// Must be known at compile time for FastLED
const unsigned LED_ROW_COUNT = 144;
const unsigned LED_COLUMN_COUNT = 10;

// Vector of shared_ptr's to Pattern Instances that will be added to the RaveLights instance
std::vector<std::shared_ptr<Pattern::AbstractPattern>> patterns{
    std::make_shared<Pattern::RandomSegments>(),         // 0
    std::make_shared<Pattern::RandomSequence>(),         // 1
    std::make_shared<Pattern::SingleStrobeFlash>(),      // 2
    std::make_shared<Pattern::MultipleStrobeFlashes>(),  // 3
    std::make_shared<Pattern::Twinkle>(),                // 4
    std::make_shared<Pattern::Comet>(),                  // 5
    std::make_shared<Pattern::MovingStrobe>(),           // 6
};

// RaveLights instance
RaveLights<LED_ROW_COUNT, LED_COLUMN_COUNT> raveLights;

void setup() {
    Serial.begin(115200);
    while (!Serial) {
        // Wait for serial port to be ready.
    }
    // Set thread config such that thread's stack suffices for RaveLights::show().
    // Use uxTaskGetStackHighWaterMark(NULL) inside thread to determine remaining stack space.
    auto thread_config = esp_pthread_get_default_config();
    thread_config.stack_size = 8192;
    ESP_ERROR_CHECK(esp_pthread_set_cfg(&thread_config));

    // Set network config
    Network::initWifiAccessPoint(WifiCredentials::ssid, WifiCredentials::password);
    // Network::connectToWifi(WifiCredentials::ssid, WifiCredentials::password);

    // esp_random() produces true random number if wifi or bluetooth is running.
    randomSeed(esp_random());

    for (auto &pattern : patterns) {
        raveLights.addPattern(pattern);
    }

    Serial.println("Testing LEDs...");
    raveLights.testLeds();

    Serial.println("Starting web server...");
    raveLights.startWebServer();

    Serial.println("Starting show loop...");
    raveLights.startShowLoop();
}

void loop() {}
