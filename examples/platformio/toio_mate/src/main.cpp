#include <Arduino.h>

#define Serial   USBSerial

#include <M5Unified.h>
#include <Toio.h>
#include <irboard.h>


static Toio toio;
static ToioCore* toiocore = nullptr;

static int toio_throttle = 10;
static int toio_steering = 50;
static int toio_direction = 0;

#define DIR_FOWORD      1
#define DIR_BACKWORD    2
#define DIR_TURN_RIGHT  4
#define DIR_TURN_LEFT   8

// Set your ssid and password here.
const char *ssid = "irBoard ESP32";
const char *password = "password";

static Irboard irboard = Irboard();

static void toio_task(void*)
{
    int period = 200;

    while (true) {

        // toio コア キューブのスキャン開始
        while(toiocore == nullptr) {
            std::vector<ToioCore*> toiocore_list = toio.scan(3);
            if (toiocore_list.size() != 0) {
            toiocore = toiocore_list.at(0);
            }
        }

        // BLE 接続
        toiocore->connect();

        int out = 0;
        int ex_out = 0;
        float roll = 0;
        float value = 0;

        while(toiocore->isConnected()) {
            unsigned long st = millis();

            toio.loop();

            uint8_t throttle = 0;
            if (toio_direction & DIR_FOWORD) {
                throttle += toio_throttle;
            }
            if (toio_direction & DIR_BACKWORD) {
                throttle -= toio_throttle;
            }

            uint8_t steering = 0;
            if (toio_direction & DIR_TURN_LEFT) {
                steering -= toio_steering;
            }
            if (toio_direction & DIR_TURN_RIGHT) {
                steering += toio_steering;
            }

            if (throttle == 0) {
                steering = 0;
            }
            toiocore->drive(throttle, steering);

            unsigned long t = period - (millis() - st);
            if (t >= st) { t = 0UL; }
            delay(t);
        }

        toiocore = nullptr;

    }
}

static void display_task(void*)
{
    ToioCore *core = nullptr;
    int direction = 0;
    bool connected = false;
    bool core_changes = true;
    bool connected_changes = true;
    bool direction_changes = true;

    int w = M5.Display.width();
    int hw = w / 2;
    int h = M5.Display.height();
    int hh = 30 + (w - 30) / 2;
    Serial.printf("w: %d, h: %d\n", w, h);

    while(true) {

        if (core_changes) {
            core_changes = false;

            M5.Display.startWrite();
            if (core == nullptr) {
                M5.Display.setColor(BLACK);
                M5.Display.setTextColor(CYAN);
            } else {
                M5.Display.setColor(CYAN);
                M5.Display.setTextColor(BLACK);
            }
            M5.Display.fillRoundRect(hw + 2, 0 + 2, hw - 4, 30 - 4, 8);
            
            M5.Display.setColor(CYAN);
            M5.Display.drawRoundRect(hw, 0, hw, 30, 8);

            M5.Display.setTextSize(2);
            M5.Display.setCursor(hw + 8, 8);
            M5.Display.print("toio");
            M5.Display.endWrite();
        }

        if (connected_changes) {
            connected_changes = false;

            M5.Display.startWrite();
            if (connected) {
                M5.Display.setColor(GREEN);
                M5.Display.setTextColor(BLACK);
            } else {
                M5.Display.setColor(BLACK);
                M5.Display.setTextColor(GREEN);
            }
            M5.Display.fillRoundRect(0 + 2, 0 + 2, hw - 4, 30 - 4, 8);
            
            M5.Display.setColor(GREEN);
            M5.Display.drawRoundRect(0, 0, hw, 30, 8);

            M5.Display.setTextSize(1);
            M5.Display.setCursor(12, 12);
            M5.Display.print("irBoard");
            M5.Display.endWrite();
        }

        if (direction_changes) {
            direction_changes = false;

            M5.Display.startWrite();
            M5.Display.setColor(ORANGE);
            M5.Display.fillRoundRect(hw - 40, hh - 20, 80, 40, 8);
            M5.Display.fillRoundRect(hw - 20, hh - 40, 40, 80, 8);

            int x = hw - 20;
            int y = hh - 20;
            M5.Display.setColor(BLACK);
            M5.Display.drawLine(x, y, x + 40, y + 40);
            y += 40;
            M5.Display.drawLine(x, y, x + 40, y - 40);

            Serial.printf("%X\n", direction);
            if ((direction & DIR_FOWORD) == 0) {
                int x = hw - 20;
                int y = hh - 40;
                M5.Display.fillRoundRect(x + 1, y + 1, 40 - 2, 20 - 2, 8);
                y += 20;
                M5.Display.fillRect(x + 1, y - 8, 40 - 2, 8);
                M5.Display.fillTriangle(hw, hh - 2, x, y - 1, x + 40, y - 1);
            }
            if ((direction & DIR_BACKWORD) == 0) {
                int x = hw - 20;
                int y = hh + 20;
                M5.Display.fillRoundRect(x + 1, y + 1, 40 - 2, 20 - 2, 8);
                M5.Display.fillRect(x + 1, y, 40 - 2, 8);
                M5.Display.fillTriangle(hw, hh + 2, x, y + 1, x + 40, y + 1);
            }
            if ((direction & DIR_TURN_RIGHT) == 0) {
                int x = hw + 20;
                int y = hh - 20;
                M5.Display.fillRoundRect(x + 1, y + 1, 20 - 2, 40 - 2, 8);
                M5.Display.fillRect(x, y + 1, 8, 40 - 2);
                M5.Display.fillTriangle(hw + 2, hh, x + 1, y, x + 1, y + 40);
            }
            if ((direction & DIR_TURN_LEFT) == 0) {
                int x = hw - 40;
                int y = hh - 20;
                M5.Display.fillRoundRect(x + 1, y + 1, 20 - 2, 40 - 2, 8);
                x += 20;
                M5.Display.fillRect(x - 8, y + 1, 8, 40 - 2);
                M5.Display.fillTriangle(hw - 2, hh, x - 1, y, x - 1, y + 40);
            }
            M5.Display.endWrite();
        }
        
        // update state
        if (core != toiocore) {
            core = toiocore;
            core_changes = true;
        }

        bool c = irboard.state() == IRBOARD_STATE_CONNECTED ? true : false;
        if (connected != c) {
            connected = c;
            connected_changes = true;
        }

        /*
        // 制御が遅れるので方向の描画はしない。
        if (direction != toio_direction) {
            direction = toio_direction;
            direction_changes = true;
        }
        */

        delay(100);
    }
}

void setup() {
    auto cfg = M5.config(); 
    M5.begin(cfg);
    Serial.begin(115200);

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

    // set default value
    irboard.setIntValue("D0", toio_throttle);
    irboard.setIntValue("D1", toio_steering);

    xTaskCreatePinnedToCore(toio_task, "toio_task", 4096, NULL, 25, NULL, APP_CPU_NUM);
    xTaskCreatePinnedToCore(display_task, "display_task", 4096, NULL, 0, NULL, APP_CPU_NUM);
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

    if (irboard.isChanged()) {
        toio_throttle = irboard.intValue("D0");
        toio_steering = irboard.intValue("D1");
        toio_direction = (irboard.boolValue("X0") ? DIR_FOWORD : 0)     + 
                         (irboard.boolValue("X1") ? DIR_BACKWORD : 0)   +
                         (irboard.boolValue("X2") ? DIR_TURN_RIGHT : 0) +
                         (irboard.boolValue("X3") ? DIR_TURN_LEFT : 0);
    }

    delay(10);
}