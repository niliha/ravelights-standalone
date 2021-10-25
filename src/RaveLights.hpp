#pragma once

#include <FastLED.h>

template <unsigned LED_ROW_COUNT, unsigned LED_COLUMN_COUNT, std::uint8_t LED_DATA_PIN = 4, EOrder RGB_ORDER = EOrder::GRB>
class RaveLights
{
public:
    RaveLights() : ledController_(FastLED.addLeds<WS2812, LED_DATA_PIN, GRB>(leds_.data(), LED_ROW_COUNT * LED_COLUMN_COUNT)){};

    void testLeds()
    {
        std::vector<CRGB> colors{CRGB::Red, CRGB::Green, CRGB::Blue};
        for (const auto color : colors)
        {
            ledController_.showColor(color);
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

private:
    std::array<CRGB, LED_ROW_COUNT * LED_COLUMN_COUNT> leds_;
    CLEDController &ledController_;
    const unsigned rowCount;
    const unsigned columnCount;

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
