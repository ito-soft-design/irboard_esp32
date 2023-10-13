#include <Arduino.h>
#include <M5Unified.h>
#include <FastLED.h>
#include <irboard.h>

// run mode
typedef enum
{
    RUN_MODE_UNKNOWN    = 0,
    RUN_MODE_NORMAL     = 1,
    RUN_MODE_SETUP      = 2,
} RunMode;

static RunMode run_mode = RUN_MODE_UNKNOWN;

// irBaord
#define SETUP_WIFI_SSID         "irBoard ESP32"
#define SETUP_WIFI_PASSWORD     "password"

Irboard *irboard = new Irboard();


// reset irboard object
static void resetIrboard()
{
    irboard->terminate();
    delay(100);
    irboard = new Irboard();
    irboard->setVerbose(true);
}

// turn on and off the led
static void set_led(bool f) {
    digitalWrite(GPIO_NUM_10, f ? LOW : HIGH);
}

// display run_mode and IP address.
static void display_info() {
    M5.Display.clear();
    M5.Display.setCursor(0, 0);
    M5.Display.println(run_mode == RUN_MODE_SETUP ? "SETUP" : "NORMAL");
    M5.Display.println("IP:");
    if (run_mode == RUN_MODE_SETUP) {
        M5.Display.println(WiFi.softAPIP());
    } else {
        M5.Display.println(WiFi.localIP());
    }
}

// set run mode
static void set_run_mode(RunMode mode)
{
    // return if nothing changes.
    if (run_mode == mode)
    {
        return;
    }

    run_mode = mode;
    switch (run_mode)
    {

    case RUN_MODE_NORMAL:
        Serial.println("Normal mode");

        resetIrboard();
        irboard->begin();

        break;

    case RUN_MODE_SETUP:
        Serial.println("Setup mode");

        resetIrboard();
        WiFi.softAP(SETUP_WIFI_SSID, SETUP_WIFI_PASSWORD);
        irboard->begin(true);

        break;
    }
    display_info();
}

void setup()
{
    auto cfg = M5.config();
    cfg.serial_baudrate = 115200;
    M5.begin(cfg);

    // LED
    pinMode(GPIO_NUM_10, OUTPUT);
    set_led(false);

    // LCD
    M5.Display.setRotation(1);
    M5.Display.setBrightness(200);
    M5.Display.fillScreen(BLACK);
    M5.Display.setTextSize(2);

    set_run_mode(RUN_MODE_NORMAL);
}

static void setup_loop()
{
    String wifi_ssid = "";
    String wifi_password = "";
    bool apply = false;
    bool cancel = false;

    // check irboard state
    irboard->update();
    set_led(irboard->state() == WL_CONNECTED);
    // Get values if it is changed.
    if (irboard->isChanged())
    {
        wifi_ssid = irboard->stringValue("D0", 20);
        wifi_password = irboard->stringValue("D10", 20);
        apply = irboard->boolValue("X0");
        cancel = irboard->boolValue("X1");
    }

    // If you press the Apply or Cancel button on the irBoard screen, turn it to normal mode.
    if (apply || cancel)
    {
        delay(500);
        resetIrboard();

        // If the SSID and password are valid, set it up to the WiFi configuration.
        if (wifi_ssid.length() != 0 && wifi_password.length() != 0)
        {
            irboard->addAP(wifi_ssid.c_str(), wifi_password.c_str());
            irboard->begin();
            unsigned long start_at = millis();
            while (irboard->state() != IRBOARD_STATE_CONNECTED)
            {
                // Brake if it can't make a connection.
                if (millis() - start_at >= 30000)
                {
                    break;
                }
                irboard->update();
                delay(100);
            }
        }

        set_run_mode(RUN_MODE_NORMAL);
    }

    delay(10);
}

void loop()
{
    static bool wifi_connected = false;

    // Do setup_loop if run_mode is RUN_MODE_SETUP
    if (run_mode == RUN_MODE_SETUP)
    {
        setup_loop();
        return;
    }

    // normal mode
    irboard->update();
    // Display IP address when wifi is connected.
    if (WiFi.status() == WL_CONNECTED) {
        if (wifi_connected == false) {
            display_info();
        }
        wifi_connected = true;
    } else {
        wifi_connected = false;
    }
    set_led(irboard->state() == WL_CONNECTED);

    // If you press the A button longer, turn it to setup mode.
    M5.update();
    if (M5.BtnA.wasReleaseFor(1000))
    {
        set_run_mode(RUN_MODE_SETUP);
    }

    delay(10);
}
