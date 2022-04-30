#pragma once

#include "ESPAsyncWebServer.h"
#include "patterns/AbstractPattern.hpp"
#define FASTLED_ESP32_I2S  // Alternative parallel output driver
#include <FastLED.h>
#include <atomic>
#include <mutex>
#include <thread>
#include <vector>

template <int PIN_COUNT, const std::array<int, PIN_COUNT> &PINS, EOrder RGB_ORDER = RGB> class RaveLights {

    struct PatternConfig {
        uint8_t brightness{255};
        unsigned color{CRGB::Purple};
        unsigned patternIndex{0};
    };

   public:
    RaveLights(const std::array<int, PIN_COUNT> &lightsPerPin, int pixelsPerLight = 144, uint8_t maxBrightness = 255)
        : PIXELS_PER_LIGHT_(pixelsPerLight), MAX_BRIGHTNESS_(maxBrightness), server_(80) {
        static_assert(PIN_COUNT == 4, "setupFastLed() is currently hardcoded to handle exactly 4 pins!");
        setupFastled(lightsPerPin);
        setupRequestHandlers();
    }

    void testLeds() {
        std::vector<CRGB> colors{CRGB::Red, CRGB::Green, CRGB::Blue};
        for (const auto color : colors) {
            auto timeBefore = millis();
            FastLED.showColor(color);
            auto passedTime = millis() - timeBefore;
            Serial.print("show() took ");
            Serial.print(passedTime);
            Serial.println(" ms");
            delay(500);
            FastLED.clear(true);
            delay(500);
        }
    }

    void addPattern(std::shared_ptr<Pattern::AbstractPattern> pattern) {
        pattern->init(PIXELS_PER_LIGHT_, LIGHT_COUNT_);
        patterns_.push_back(pattern);
    }

    void startWebServer() { server_.begin(); }

    void show() {
        while (!stopShowLoop_) {
            FastLED.clear(false);
            unsigned long offDurationMs =
                patterns_[currentPatternConfig_.patternIndex]->perform(leds_, currentPatternConfig_.color);
            unsigned long currentTimeMs = millis();
            do {
                {  // Begin of scope guarded by mutex
                    // Leave waiting loop prematurely if pattern change is requested by asynchronous web server thread
                    std::lock_guard<std::mutex> lockGuard(isPatternUpdatePendingMutex_);
                    if (isPatternUpdatePending_) {
                        currentPatternConfig_.patternIndex = nextPatternConfig_.patternIndex;
                        isPatternUpdatePending_ = false;
                        break;
                    }
                }  // End of scope guarded by mutex
            } while (millis() - currentTimeMs < offDurationMs);
            updatePatternConfig();
        }
    }

    void startShowLoop() { showLoopThread_ = std::thread(&RaveLights::show, this); }

    void stopShowLoop() {
        stopShowLoop_ = true;
        showLoopThread_.join();  // wait for thread to finish
        stopShowLoop_ = false;
    }

   private:
    const int PIXELS_PER_LIGHT_;
    const uint8_t MAX_BRIGHTNESS_;
    int PIXEL_COUNT_;
    int LIGHT_COUNT_;

    std::vector<CRGB> leds_;
    std::vector<std::shared_ptr<Pattern::AbstractPattern>> patterns_;
    bool isPatternUpdatePending_{false};
    std::mutex isPatternUpdatePendingMutex_;
    AsyncWebServer server_;
    struct PatternConfig currentPatternConfig_;
    struct PatternConfig nextPatternConfig_;
    std::atomic_bool stopShowLoop_{false};
    std::thread showLoopThread_;

    void setupFastled(const std::array<int, PIN_COUNT> &lightsPerPin) {
        // Allocate led buffer
        LIGHT_COUNT_ = std::accumulate(lightsPerPin.begin(), lightsPerPin.end(), 0);
        PIXEL_COUNT_ = LIGHT_COUNT_ * PIXELS_PER_LIGHT_;
        leds_.resize(PIXEL_COUNT_);
        // We can't use a loop here since addLeds() template parameters must be known at
        // compile-time
        int pixelOffset = 0;
        if (lightsPerPin[0] > 0) {
            FastLED.addLeds<WS2812, PINS[0], RGB_ORDER>(leds_.data(), pixelOffset, lightsPerPin[0] * PIXELS_PER_LIGHT_);
            pixelOffset += lightsPerPin[0] * PIXELS_PER_LIGHT_;
        }
        if (lightsPerPin[1] > 0) {
            FastLED.addLeds<WS2812, PINS[1], RGB_ORDER>(leds_.data(), pixelOffset, lightsPerPin[1] * PIXELS_PER_LIGHT_);
            pixelOffset += lightsPerPin[1] * PIXELS_PER_LIGHT_;
        }
        if (lightsPerPin[2] > 0) {
            FastLED.addLeds<WS2812, PINS[2], RGB_ORDER>(leds_.data(), pixelOffset, lightsPerPin[2] * PIXELS_PER_LIGHT_);
            pixelOffset += lightsPerPin[2] * PIXELS_PER_LIGHT_;
        }
        if (lightsPerPin[3] > 0) {
            FastLED.addLeds<WS2812, PINS[3], RGB_ORDER>(leds_.data(), pixelOffset, lightsPerPin[3] * PIXELS_PER_LIGHT_);
            pixelOffset += lightsPerPin[3] * PIXELS_PER_LIGHT_;
        }
        // Set maximum brightness (0 - 255)
        // Currently the leds don't work well at highest brightness
        uint8_t safe_brightness = min(MAX_BRIGHTNESS_, (uint8_t)200);
        FastLED.setBrightness(safe_brightness);
    }

    void updatePatternConfig() {
        currentPatternConfig_ = nextPatternConfig_;
        FastLED.setBrightness(currentPatternConfig_.brightness);
    }

    void setupRequestHandlers() {
        server_.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
            request->send_P(200, "text/html", "Documentation in process...");
        });
        setupPatternRequestHandler();
        setupBrightnessRequestHandler();
        setupColorRequestHandler();
    }

    void setupPatternRequestHandler() {
        server_.on("/pattern", HTTP_GET, [this](AsyncWebServerRequest *request) {
            bool hasError = false;
            int patternIndex = 0;
            if (request->hasParam("value")) {
                patternIndex = request->getParam("value")->value().toInt();
                if (patternIndex < 0 || patternIndex >= patterns_.size()) {
                    hasError = true;
                }
            } else {
                hasError = true;
            }
            if (hasError) {
                request->send(200, "text/plain", "Error. Could not update pattern to #" + String(patternIndex));
            } else {
                nextPatternConfig_.patternIndex = patternIndex;
                request->send(200, "text/plain", "OK. Pattern Updated to #" + String(patternIndex));
                {
                    std::lock_guard<std::mutex> lockGuard(isPatternUpdatePendingMutex_);
                    isPatternUpdatePending_ = true;
                }
            }
        });
    }

    void setupBrightnessRequestHandler() {
        server_.on("/brightness", HTTP_GET, [this](AsyncWebServerRequest *request) {
            bool hasError = false;
            int brightness = 0;
            if (request->hasParam("value")) {
                brightness = request->getParam("value")->value().toInt();
                if (brightness < 0 || brightness > 255) {
                    hasError = true;
                }
            } else {
                hasError = true;
            }
            if (hasError) {
                request->send(200, "text/plain", "Error. Could not update brightness to " + String(brightness));
            } else {
                nextPatternConfig_.brightness = brightness;
                request->send(200, "text/plain", "OK. Brightness Updated to " + String(brightness));
            }
        });
    }

    void setupColorRequestHandler() {
        server_.on("/color", HTTP_GET, [this](AsyncWebServerRequest *request) {
            bool hasError = false;
            int color = 0;
            if (request->hasParam("value")) {
                String colorString = request->getParam("value")->value();
                color = strtol(colorString.c_str(), NULL, 16);
                if (color < 0 || color > 0xffffff) {
                    hasError = true;
                }
            } else {
                hasError = true;
            }
            if (hasError) {
                request->send(200, "text/plain", "Error. Could not update color to " + String(color));
            } else {
                nextPatternConfig_.color = color;
                request->send(200, "text/plain", "OK. Color Updated to 0x" + String(color));
            }
        });
    }
};
