#include <Arduino.h>
#include <M5Unified.h>
#include <FastLED.h>
#include <irboard.h>

// If you want to act as an ap mode, uncomment here.
// #define ACTS_AS_AP_MODE

// Set your ssid and password here.
const char *ssid = "irBoard ESP32";
const char *password = "password";

// LED
#define NUM_LEDS      1
static CRGB leds[NUM_LEDS];

static Irboard irboard = Irboard();

static short value_a = 0;
static short color_r = 0;
static short color_g = 0;
static short color_b = 0;

#define TEXT_MAX_LENGTH     32
static String text = "Hello, world.";

void config_board() {
    switch(M5.getBoard()) {
    case m5gfx::board_t::board_M5StickC:
        pinMode(GPIO_NUM_10, OUTPUT);   // LED
        M5.Display.setRotation(1);
        break;
    case m5gfx::board_t::board_M5Atom:
    case m5gfx::board_t::board_M5StampPico:
        FastLED.addLeds<WS2811, GPIO_NUM_27, RGB>(leds, NUM_LEDS);
        FastLED.setBrightness(255 * 15 / 100);
        pinMode (GPIO_NUM_0, OUTPUT);
        digitalWrite (GPIO_NUM_0, LOW);
        break;
    }
}

void setRGBLed(CRGB color) {
    // change RGB to GRB
    uint8_t t = color.r;
    color.r = color.g;
    color.g = t;
    leds[0] = color;
    FastLED.show();
}

void setLed(int flag) {
    if (M5.getBoard() == m5gfx::board_t::board_M5StickC) {
        digitalWrite(GPIO_NUM_10, flag);   
    }
}

void setup() {
    auto cfg = M5.config(); 
    M5.begin(cfg);                // M5STACK INITIALIZE
    Serial.begin(115200);
    M5.Display.setBrightness(200);    // BRIGHTNESS = MAX 255
    M5.Display.fillScreen(BLACK);     // CLEAR SCREEN
    M5.Display.setTextSize(2);
    
    config_board();

    // show IP address to the terminal when it's established connection.
    irboard.setVerbose(true);
    irboard.setStringValue("D4", text, TEXT_MAX_LENGTH);

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
}

void display_info()
{
    CRGB color;

    switch(M5.getBoard()) {
    case m5gfx::board_t::board_M5StickC:
    case m5gfx::board_t::board_M5Stack:
        {
            uint16_t bg_color = M5.Display.color565(color_r, color_g, color_b);
            M5.Display.fillScreen(bg_color);
            M5.Display.setTextColor(WHITE, bg_color);

            M5.Display.setCursor(0, 0);
            M5.Display.println("IP:");
            M5.Display.print("  ");
#ifdef ACTS_AS_AP_MODE
            M5.Display.println(WiFi.softAPIP());
#else
            M5.Display.println(WiFi.localIP());
#endif    
            M5.Display.println("");
            M5.Display.print("A: ");
            M5.Display.println(value_a);

            M5.Display.print("T: ");
            M5.Display.println(text);
        }
        break;

    case m5gfx::board_t::board_M5Atom:
    case m5gfx::board_t::board_M5StampPico:
        {
            color.r = color_r;
            color.g = color_g;
            color.b = color_b;
            setRGBLed(color);
        }
        break;
    }
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
    setLed(irboard.boolValue("X0") ? LOW : HIGH);

    value_a = irboard.shortValue("D0");

    color_r = irboard.shortValue("D1");
    color_g = irboard.shortValue("D2");
    color_b = irboard.shortValue("D3");

    text = irboard.stringValue("D4", TEXT_MAX_LENGTH);

    if (irboard.isChanged()) {
        display_info();
    }
}
