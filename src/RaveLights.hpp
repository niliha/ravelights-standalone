#pragma once

#include <FastLED.h>
#include "Pattern.hpp"
#include <vector>

template <unsigned LED_ROW_COUNT, unsigned LED_COLUMN_COUNT, std::uint8_t LED_DATA_PIN = 4, EOrder RGB_ORDER = EOrder::GRB>
class RaveLights
{
public:
    RaveLights() : leds_(LED_ROW_COUNT * LED_COLUMN_COUNT, CRGB::Black)
    {
        FastLED.addLeds<WS2812, LED_DATA_PIN, GRB>(leds_.data(), LED_ROW_COUNT * LED_COLUMN_COUNT);
        testLeds();
    }

    void testLeds()
    {
        std::vector<CRGB> colors{CRGB::Red, CRGB::Green, CRGB::Blue};
        for (const auto color : colors)
        {
            FastLED.showColor(color);
            delay(500);
        }
    }

    void initWifi(bool createAccessPoint, std::string wifiSsid, std::string wifiPassword)
    {
        if (createAccessPoint)
        {
            initWifiAccessPoint(wifiSsid, wifiPassword);
        }
        else
        {
            connectToWifi(wifiSsid, wifiPassword);
        }
    }

    void addPattern(std::shared_ptr<Pattern::AbstractPattern> pattern)
    {
        pattern->init(LED_ROW_COUNT, LED_COLUMN_COUNT);
        patterns_.push_back(pattern);
    }

    void startShowLoop()
    {
        while (true)
        {
            FastLED.clear(true);
            unsigned offDuration = (*patterns_[currentPattern_]).perform(leds_, currentColor_);
            delay(offDuration);
        }
    }

private:
    std::vector<CRGB> leds_;
    std::vector<std::shared_ptr<Pattern::AbstractPattern>> patterns_;
    // pattern parameters
    uint8_t currentBrightness_ = 255;
    uint8_t nextBrightness_ = 255;
    int currentColor_ = CRGB::Purple;
    int nextColor_ = CRGB::Purple;
    unsigned currentPattern_ = 0;
    unsigned nextPattern_ = 0;

    bool connectToWifi(std::string wifiSsid, std::string wifiPassword)
    {
        WiFi.begin(wifiSsid.c_str(), wifiPassword.c_str());
        Serial.println("Connecting to WiFi");
        // Wait for connection
        Serial.print("Connecting");
        for (unsigned i = 0; i <= 20; i++)
        {
            if (WiFi.status() == WL_CONNECTED)
            {
                Serial.print("SUCCESS! IP address:  ");
                Serial.println(WiFi.localIP());
                return true;
            }
            delay(500);
            Serial.print(".");
        }
        Serial.println("ERROR! Connection failed.");
        return false;
    }

    void initWifiAccessPoint(std::string wifiSsid, std::string wifiPassword)
    {
        Serial.println("Setting up access point...");
        WiFi.mode(WIFI_AP); // Changing ESP32 wifi mode to AccessPoint
        WiFi.softAP(wifiSsid.c_str(), wifiPassword.c_str());
        IPAddress ipAddress = WiFi.softAPIP();
        Serial.print("AP IP address: ");
        Serial.println(ipAddress); // Default IP is 192.168.4.1
    }
};
