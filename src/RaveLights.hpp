#pragma once

#include "ESPAsyncWebServer.h"
#include "Pattern.hpp"
#include <FastLED.h>
#include <mutex>
#include <vector>

template <unsigned LED_ROW_COUNT, unsigned LED_COLUMN_COUNT, std::uint8_t LED_DATA_PIN = 4,
          EOrder RGB_ORDER = EOrder::GRB>
class RaveLights {
   public:
    RaveLights() : leds_(LED_ROW_COUNT * LED_COLUMN_COUNT, CRGB::Black), server_(80) {
        FastLED.addLeds<WS2812, LED_DATA_PIN, GRB>(leds_.data(), LED_ROW_COUNT * LED_COLUMN_COUNT);
        delay(100);
        setupRequestHandlers();
    }

    void testLeds() {
        std::vector<CRGB> colors{CRGB::Red, CRGB::Green, CRGB::Blue, CRGB::Black};
        for (const auto color : colors) {
            FastLED.showColor(color);
            delay(500);
        }
    }

    void addPattern(std::shared_ptr<Pattern::AbstractPattern> pattern) {
        pattern->init(LED_ROW_COUNT, LED_COLUMN_COUNT);
        patterns_.push_back(pattern);
    }

    void startWebServer() { server_.begin(); }

    void startShowLoop() {
        while (true) {
            FastLED.clear(true);
            unsigned long offDurationMs = patterns_[currentPatternIndex]->perform(leds_, currentColor_);
            updatePatternParameters();
            unsigned long currentTimeMs = millis();
            do {
                {  // Begin of scope guarded by mutex
                    // Leave while loop prematurely if pattern change is requested by asynchronous web server thread
                    std::lock_guard<std::mutex> lockGuard(isPatternUpdatePendingMutex_);
                    if (isPatternUpdatePending_) {
                        updatePatternParameters();
                        isPatternUpdatePending_ = false;
                        break;
                    }
                }  // End of scope guarded by mutex
            } while (millis() - currentTimeMs < offDurationMs);
        }
    }

   private:
    std::vector<CRGB> leds_;
    std::vector<std::shared_ptr<Pattern::AbstractPattern>> patterns_;
    // pattern parameters
    uint8_t currentBrightness_{255};
    uint8_t nextBrightness_{255};
    int currentColor_{CRGB::Purple};
    int nextColor_{CRGB::Purple};
    unsigned currentPatternIndex{0};
    unsigned nextPatternIndex = {0};
    bool isPatternUpdatePending_{false};
    // We need a mutex because the web server thread and the main thread are
    // both writing to isPatternUpdatepending_
    std::mutex isPatternUpdatePendingMutex_;
    AsyncWebServer server_;

    void updatePatternParameters() {
        currentBrightness_ = nextBrightness_;
        currentColor_ = nextColor_;
        currentPatternIndex = nextPatternIndex;
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
                request->send(200, "text/plain", "Error. Could not update pattern to #" + patternIndex);
            } else {
                nextPatternIndex = patternIndex;
                request->send(200, "text/plain", "OK. Pattern Updated to #" + patternIndex);
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
                request->send(200, "text/plain", "Error. Could not update brightness to " + brightness);
            } else {
                nextBrightness_ = brightness;
                request->send(200, "text/plain", "OK. Brightness Updated to " + brightness);
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
                request->send(200, "text/plain", "Error. Could not update color to " + color);
            } else {
                nextColor_ = color;
                request->send(200, "text/plain", "OK. Color Updated to 0x" + color);
            }
        });
    }
};
