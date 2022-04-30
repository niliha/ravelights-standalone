#include "RaveLights.hpp"
#include "network/Network.hpp"
#include "network/WifiCredentials.hpp"
#include "patterns/AbstractPattern.hpp"
#include "patterns/Comet.hpp"
#include "patterns/MovingStrobe.hpp"
#include "patterns/MultipleStrobeFlashes.hpp"
#include "patterns/RandomSegments.hpp"
#include "patterns/RandomSequence.hpp"
#include "patterns/SingleStrobeFlash.hpp"
#include "patterns/Twinkle.hpp"

#include <Arduino.h>
#include <FastLED.h>
#include <esp_pthread.h>

/* BEGIN USER CONFIG */
// Specify the maximum number of pins to which lights are to be connected in a specific scenario.
// Right now the PixelDriver class is fixed to 4.
const int MAX_PIN_COUNT = 4;
// Specify the amount of individually addressable pixels per "light"
const int PIXELS_PER_LIGHT = 144;
// Specify the GPIO pins to which lights are connected.
extern constexpr std::array<int, MAX_PIN_COUNT> PINS = {19, 18, 22, 21};
// For each pin, specify how many lights are connected.
// If there are no lights connected to a specific, set lightCount to 0.
std::array<int, MAX_PIN_COUNT> lightsPerPin = {5, 5, 0, 0};
const EOrder RGB_ORDER = EOrder::RGB;
/* END USER CONFIG */

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

    // Setup and start RaveLights
    RaveLights<MAX_PIN_COUNT, PINS, RGB_ORDER> raveLights(lightsPerPin, PIXELS_PER_LIGHT);
    for (auto &pattern : patterns) {
        raveLights.addPattern(pattern);
    }

    Serial.println("Testing LEDs...");
    raveLights.testLeds();

    Serial.println("Starting web server...");
    raveLights.startWebServer();

    Serial.println("Starting show loop...");
    raveLights.startShowLoop();

    while (true) {
    }
}

void loop() {}
