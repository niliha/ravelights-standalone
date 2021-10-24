#include "ESPAsyncWebServer.h"
#include "WiFi.h"
#include <Arduino.h>
#include <FastLED.h>
#include <vector>

// Forward declarations
void pickColumnsRandomly(int *columns, unsigned numOfCols);
int flipPixelVertically(int pixel, int column);
void performMeteorColumnDown(unsigned column);
void performMeteorColumn(unsigned column);
int deriveRandomColor(int color);

// Constants
const int NUM_ROWS = 30;
const int NUM_COLS = 5;
const int NUM_LEDS = NUM_ROWS * NUM_COLS;
const byte DATA_PIN = 4;

const int DURATION_ON_MS_MIN = 100;
const int DURATION_ON_MS_MAX = 500;
const int DURATION_OFF_MS_MIN = 300;
const int DURATION_OFF_MS_MAX = 1000;

// Pattern parameters
byte brightnessCurrent = 255;
byte brightnessNext = 255;
int colorHexCurrent = 0xff00ff;
int colorHexNext = 0xff00ff;

unsigned patternCurrent = 0;
unsigned patternNext = 0;
unsigned speedDividerCurrent = 1;
unsigned speedDividerNext = 1;
CRGB leds[NUM_LEDS];

// Webserver
const bool CREATE_ACCESS_POINT = true;
const char *WIFI_SSID = "ssid";
const char *WIFI_PASSWORD = "pwd";
const char *AP_SSID = "esp32";
const char *AP_PASSWORD = "partylight";
AsyncWebServer server(80);

void performPattern1(); // forward declaration
void performPattern2();
void performPattern3();
void performPattern4();
void performPattern5();
void performPattern6();
const int NUM_PATTERNS = 6;

typedef void (*VoidFunctionPointer)();
VoidFunctionPointer patterns[NUM_PATTERNS] = {performPattern1, performPattern2,
                                              performPattern3, performPattern4,
                                              performPattern5, performPattern6};

// Pattern 1 config
// For each amount of columns that should light up at the same time, we
// configure a likelihood between 0 (never) and 10 (most often).
const unsigned PATTERN1_COLUMN_PROBABILITIES[NUM_COLS + 1] = {2, 4, 8, 7, 5, 3};
unsigned pattern1ColumnDistribution[(NUM_COLS + 1) * 10];
unsigned pattern1ColumnDistributionLength = 0;

// Pattern 2 config
// For each amount of columns that should light up at the same time, we
// configure a likelihood between 0 (never) and 10 (most often).
const unsigned PATTERN2_COLUMN_PROBABILITIES[NUM_COLS + 1] = {1, 3, 5,
                                                              4, 6, 10};
unsigned pattern2ColumnDistribution[(NUM_COLS + 1) * 10];
unsigned pattern2ColumnDistributionLength = 0;

void initColumnProbabilityDistribution(unsigned *distribution,
                                       unsigned *distributionLength,
                                       const unsigned *probabilities)
{
  // for every amount of columns that should light up at the same time
  // (0,1,2,..)
  for (unsigned i = 0; i < NUM_COLS + 1; i++)
  {
    for (unsigned j = 0; j < probabilities[i]; j++)
    {
      distribution[*distributionLength] = i;
      (*distributionLength)++;
    }
  }
}

void initLedPatterns()
{
  // pattern1
  initColumnProbabilityDistribution(pattern1ColumnDistribution,
                                    &pattern1ColumnDistributionLength,
                                    PATTERN1_COLUMN_PROBABILITIES);

  // pattern2
  initColumnProbabilityDistribution(pattern2ColumnDistribution,
                                    &pattern2ColumnDistributionLength,
                                    PATTERN2_COLUMN_PROBABILITIES);

  // DEBUG
  Serial.print("Pattern 1 Distr length: ");
  Serial.print(pattern1ColumnDistributionLength);
  Serial.println();
  for (unsigned i = 0; i < pattern1ColumnDistributionLength; i++)
  {
    Serial.println(pattern1ColumnDistribution[i]);
  }

  Serial.print("Pattern 2 Distr length: ");
  Serial.print(pattern2ColumnDistributionLength);
  Serial.println();
  for (unsigned i = 0; i < pattern2ColumnDistributionLength; i++)
  {
    Serial.println(pattern2ColumnDistribution[i]);
  }
}
unsigned getStartIndexOfColumn(unsigned column) { return column * NUM_ROWS; }

unsigned getEndIndexOfColumn(unsigned column)
{
  return getStartIndexOfColumn(column) + NUM_ROWS - 1;
}

void performPattern1()
{
  // segments on each columns blinking up
  FastLED.clear();
  unsigned numOfColsToLightUp =
      pattern1ColumnDistribution[random(pattern1ColumnDistributionLength)];

  // Switch up color in 10 percent of cases
  int color = colorHexCurrent;
  if (random(0, 100) < 10)
  {
    color = deriveRandomColor(colorHexCurrent);
  }

  for (unsigned i = 0; i < numOfColsToLightUp; i++)
  {
    unsigned columnToLightUp = random(NUM_COLS);

    unsigned pixelIntervalLength = 6;
    unsigned pixelIntervalStart = getStartIndexOfColumn(columnToLightUp) +
                                  random(0, NUM_ROWS - pixelIntervalLength);
    unsigned pixelIntervalEnd = pixelIntervalStart + pixelIntervalLength;
    for (unsigned j = pixelIntervalStart;
         j < pixelIntervalStart + pixelIntervalLength + 1; j++)
    {
      // leds[j] = invertColor(colorHexCurrent);
      // leds[j] = deriveRandomColor(colorHexCurrent);
      leds[j] = color;
    }
  }

  FastLED.show();
  unsigned durationOn = random(DURATION_ON_MS_MIN, DURATION_ON_MS_MAX + 1);
  delay(durationOn);
  FastLED.clear();
  FastLED.show();
  unsigned durationOff = random(DURATION_OFF_MS_MIN, DURATION_OFF_MS_MAX + 1);
  // delay(durationOff);
  delay(durationOn);
}

void performPattern2()
{
  // single strobe once in a while

  FastLED.clear();
  unsigned numOfColsToLightUp =
      pattern2ColumnDistribution[random(pattern2ColumnDistributionLength)];

  int columnsToLightUp[NUM_COLS];
  pickColumnsRandomly(columnsToLightUp, numOfColsToLightUp);
  // DEBUG
  Serial.println("COlumns to light up:");
  for (int i = 0; i < numOfColsToLightUp; i++)
  {
    Serial.println(columnsToLightUp[i]);
  }

  for (unsigned i = 0; i < numOfColsToLightUp; i++)
  {
    unsigned columnToLightUp = columnsToLightUp[i];
    for (unsigned j = getStartIndexOfColumn(columnToLightUp);
         j < getEndIndexOfColumn(columnToLightUp) + 1; j++)
    {
      leds[j] = colorHexCurrent;
    }
  }

  FastLED.show();
  unsigned durationOn = random(50, 200);
  delay(durationOn);
  FastLED.clear();
  FastLED.show();
  unsigned durationOff = random(1000, 10000);
  delay(durationOff);
}

void performPattern3()
{
  // flash up every bar in a random sequence

  FastLED.clear();
  unsigned numOfColsToLightUp = NUM_COLS;

  int columnsToLightUp[NUM_COLS];
  pickColumnsRandomly(columnsToLightUp, numOfColsToLightUp);

  for (unsigned i = 0; i < numOfColsToLightUp; i++)
  {
    unsigned columnToLightUp = columnsToLightUp[i];
    for (unsigned j = getStartIndexOfColumn(columnToLightUp);
         j < getEndIndexOfColumn(columnToLightUp) + 1; j++)
    {
      leds[j] = colorHexCurrent;
    }
    FastLED.show();
    unsigned durationOn = random(30, 60);
    delay(durationOn);
    FastLED.clear();
    FastLED.show();
  }
  signed durationOff = random(700, 3000);
  delay(durationOff);
}

void performPattern4()
{
  // solid strobe
  FastLED.clear();
  unsigned numOfColsToLightUp =
      pattern1ColumnDistribution[random(pattern2ColumnDistributionLength)];

  int columnsToLightUp[NUM_COLS];

  unsigned numOfFlashes = random(1, 15);
  for (unsigned j = 1; j < numOfFlashes; j++)
  {
    pickColumnsRandomly(columnsToLightUp, numOfColsToLightUp);
    for (unsigned i = 0; i < numOfColsToLightUp; i++)
    {
      unsigned columnToLightUp = columnsToLightUp[i];
      for (unsigned j = getStartIndexOfColumn(columnToLightUp);
           j < getEndIndexOfColumn(columnToLightUp) + 1; j++)
      {
        leds[j] = colorHexCurrent;
      }
    }
    FastLED.show();
    unsigned flashDuration = 20;
    delay(flashDuration);
    FastLED.clear();
    FastLED.show();
    delay(flashDuration);
  }
  FastLED.clear();
  FastLED.show();
  signed durationOff = random(700, 15000);
  delay(durationOff);
}

void performPattern5()
{
  FastLED.clear();
  int pixelIndex = random(NUM_LEDS);
  if (random(0, 2) == 1)
  {
    leds[(pixelIndex - 1) % NUM_LEDS] = colorHexCurrent;
  }

  leds[pixelIndex] = colorHexCurrent;
  if (random(0, 2) == 1)
  {
    leds[(pixelIndex + 1) % NUM_LEDS] = colorHexCurrent;
  }
  FastLED.show();
  delay(random(0, 50));
}

void performPattern6()
{
  FastLED.clear();
  // meteor rain
  unsigned columnToLightUp = random(0, NUM_COLS);
  // performMeteorColumn(columnToLightUp);
  if (random(0, 2) == 1)
  {
    performMeteorColumnDown(columnToLightUp);
  }
  else
  {
    performMeteorColumn(columnToLightUp);
  }
  // delay(500);
}

void performMeteorColumn(unsigned column)
{
  // meteor rain
  byte meteorSize = 4;
  byte meteorTrailDecay = 64;
  boolean meteorRandomDecay = true;
  int speedDelay = 5;

  for (int i = getStartIndexOfColumn(column);
       i < 2 * (getEndIndexOfColumn(column) + 1); i++)
  {

    // fade brightness all LEDs one step
    for (int j = getStartIndexOfColumn(column);
         j < getEndIndexOfColumn(column) + 1; j++)
    {
      if ((!meteorRandomDecay) || (random(10) > 5))
      {
        leds[j].fadeToBlackBy(meteorTrailDecay);
        leds[(j + 2 * (NUM_ROWS)) % NUM_LEDS].fadeToBlackBy(meteorTrailDecay);
      }
    }

    // draw meteor
    for (int j = 0; j < meteorSize; j++)
    {
      if ((i - j < getEndIndexOfColumn(column) + 1) &&
          (i - j >= getStartIndexOfColumn(column)))
      {
        leds[i - j] = colorHexCurrent;
        leds[(i - j + 2 * (NUM_ROWS)) % NUM_LEDS] = colorHexCurrent;
      }
    }

    FastLED.show();
    delay(speedDelay);
  }
}

void performMeteorColumnDown(unsigned column)
{
  // meteor rain
  byte meteorSize = 4;
  byte meteorTrailDecay = 64;
  boolean meteorRandomDecay = true;
  int speedDelay = 5;

  for (int i = getStartIndexOfColumn(column);
       i < 2 * (getEndIndexOfColumn(column) + 1); i++)
  {

    // fade brightness all LEDs one step
    for (int j = getStartIndexOfColumn(column);
         j < getEndIndexOfColumn(column) + 1; j++)
    {
      if ((!meteorRandomDecay) || (random(10) > 5))
      {
        leds[flipPixelVertically(j, column)].fadeToBlackBy(meteorTrailDecay);
        leds[(flipPixelVertically(j, column) + 2 * (NUM_ROWS)) % NUM_LEDS]
            .fadeToBlackBy(meteorTrailDecay);
      }
    }

    // draw meteor
    for (int j = 0; j < meteorSize; j++)
    {
      if ((i - j < getEndIndexOfColumn(column) + 1) &&
          (i - j >= getStartIndexOfColumn(column)))
      {
        leds[flipPixelVertically(i - j, column)] = colorHexCurrent;
        leds[(flipPixelVertically(i - j, column) + 2 * (NUM_ROWS)) % NUM_LEDS] =
            colorHexCurrent;
      }
    }

    FastLED.show();
    delay(speedDelay);
  }
}

int flipPixelVertically(int pixel, int column)
{
  return getEndIndexOfColumn(column) - pixel + getStartIndexOfColumn(column);
}

bool isIntInArray(int integer, int *arr, unsigned len)
{
  for (unsigned i = 0; i < len; i++)
  {
    if (arr[i] == integer)
    {
      return true;
    }
  }
  return false;
}

void pickColumnsRandomly(int *columns, unsigned numOfCols)
{
  for (unsigned i = 0; i < numOfCols; i++)
  {
    columns[i] = -1;
  }

  for (unsigned i = 0; i < numOfCols; i++)
  {
    unsigned columnToLightUp;
    do
    {
      columnToLightUp = random(NUM_COLS);
    } while (isIntInArray(columnToLightUp, columns, numOfCols));
    columns[i] = columnToLightUp;
  }
}

void testLeds()
{
  Serial.println("Testing Leds...");
  for (int i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = CRGB(127, 0, 0);
  }
  FastLED.show();
  delay(500);
  for (int i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = CRGB(0, 127, 0);
  }
  FastLED.show();
  delay(500);
  for (int i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = CRGB(0, 0, 127);
  }
  FastLED.show();
  delay(500);
  for (int i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = CRGB(127, 0, 0);
  }
  FastLED.show();
}

bool connectToWifi(void)
{
  bool state = true;
  int i = 0;

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.println("");
  Serial.println("Connecting to WiFi");

  // Wait for connection
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
    if (i > 20)
    {
      state = false;
      break;
    }
    i++;
  }
  if (state)
  {
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(WIFI_SSID);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }
  else
  {
    Serial.println("");
    Serial.println("Connection failed.");
  }

  return state;
}

int invertColor(int color) { return 0xFFFFFF - color; }

int deriveRandomColor(int color) { return (color ^ random(0, 0xffffff)); }

void initWifiAccessPoint()
{
  Serial.println("Configuring access point...");
  WiFi.mode(WIFI_AP); // Changing ESP32 wifi mode to AccessPoint
  WiFi.softAP(AP_SSID, AP_PASSWORD);
  IPAddress ipAddress = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(ipAddress); // Default IP is 192.168.4.1
}

void updateLEDParameters()
{
  brightnessCurrent = brightnessNext;
  // Applies this brightness to all subsequent values
  FastLED.setBrightness(brightnessCurrent);
  colorHexCurrent = colorHexNext;
  patternCurrent = patternNext;
  speedDividerCurrent = speedDividerNext;
}

void initWebserver()
{
  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/html", "Documentation in process..."); });

  // Send a GET request to <ESP_IP>/brightness?value=<inputMessage>
  server.on("/brightness", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              // GET input1 value on <ESP_IP>/brightness?value=<inputMessage>
              if (request->hasParam("value"))
              {
                String inputValue = request->getParam("value")->value();
                brightnessNext = inputValue.toInt();
                request->send(200, "text/plain",
                              "OK. Brightness Updated to " + inputValue);
              }

              else
              {
                request->send(200, "text/plain", "Error. Could not update brightness");
              }
            });

  // Send a GET request to
  // <ESP_IP>/color?r=<inputValueRed>&g=<inputValueGreen>&b=<inputValueBlue>
  server.on("/color", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              if (request->hasParam("value"))
              {
                String inputValue = request->getParam("value")->value();
                colorHexNext = strtol(inputValue.c_str(), NULL, 16);
                request->send(200, "text/plain", "OK. Color Updated to 0x" + inputValue);
              }
              else
              {
                request->send(200, "text/plain", "Error. Could not update color");
              }
            });

  server.on("/pattern", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              if (request->hasParam("value"))
              {
                String inputValue = request->getParam("value")->value();
                patternNext = inputValue.toInt() - 1;
                request->send(200, "text/plain", "OK. Pattern Updated to #" + inputValue);
              }
              else
              {
                request->send(200, "text/plain", "Error. Could not update pattern");
              }
            });

  // Start server
  server.begin();
}

void setup()
{
  randomSeed(analogRead(2));
  Serial.begin(115200);
  FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS);
  testLeds();
  initLedPatterns();
  if (CREATE_ACCESS_POINT)
  {
    initWifiAccessPoint();
  }
  else
  {
    connectToWifi();
  }
  initWebserver();
}

void loop()
{
  updateLEDParameters();
  patterns[patternCurrent]();
}