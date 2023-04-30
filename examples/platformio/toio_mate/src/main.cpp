#include <Arduino.h>
#include <M5Unified.h>
#include <FastLED.h>
#include <Toio.h>
#include <irboard.h>


static Toio toio;
static ToioCore* toiocore = nullptr;

// Set your ssid and password here.
const char *ssid = "irBoard ESP32";
const char *password = "password";

// LED
#define NUM_LEDS      25
static CRGB leds[NUM_LEDS];

static Irboard irboard = Irboard();


void setup() {
    auto cfg = M5.config(); 
    M5.begin(cfg);                // M5STACK INITIALIZE
    Serial.begin(115200);

    FastLED.addLeds<WS2811, GPIO_NUM_27, RGB>(leds, NUM_LEDS);
    FastLED.setBrightness(255 * 15 / 100);
    pinMode (GPIO_NUM_0, OUTPUT);
    digitalWrite (GPIO_NUM_0, LOW);

    // show IP address to the terminal when it's established connection.
    irboard.setVerbose(true);

    // configure wifi connection
#ifdef ACTS_AS_AP_MODE
    WiFi.softAP(ssid, password);
    Serial.print(WiFi.softAPIP());
     // If set true, it acts as the ap mode.
    irboard.begin(true);
#else
    irboard.addAP(ssid, password);
    irboard.begin();
#endif
    // for permit writing
    irboard.setShortValue("SD19", 1);
}

void loop() {
    irboard.update();
    if (irboard.state() == IRBOARD_STATE_INITIAL) {
        // If the state is IRBOARD_STATE_INITIAL, the connection is closed. You should set SSID again.
#ifdef ACTS_AS_AP_MODE
        WiFi.softAP(ssid, password);
#else
        irboard.addAP(ssid, password);
#endif
    }

    M5.update();
    bool pressed = M5.BtnA.isPressed() || M5.BtnB.isPressed() || M5.BtnC.isPressed();

    irboard.setBoolValue("Y0", pressed);

    if (irboard.isChanged()) {
    }
}