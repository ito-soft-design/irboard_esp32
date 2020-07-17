#include <M5Stack.h>
#include <irboard.h>

// If you want to act as an ap mode, uncomment here.
// #define ACTS_AS_AP_MODE

// Set your ssid and password here.
const char *ssid = "ssid";
const char *password = "password";

Irboard irboard = Irboard();

short value_a = 0;
short color_r = 0;
short color_g = 0;
short color_b = 0;

void setup() {
    M5.begin();                   // M5STACK INITIALIZE
    Serial.begin(115200);
    M5.Lcd.setBrightness(200);    // BRIGHTNESS = MAX 255
    M5.Lcd.fillScreen(BLACK);     // CLEAR SCREEN
    M5.Lcd.setTextSize(2);

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
}

void display_info()
{
    M5.Lcd.fillScreen(M5.Lcd.color565(color_r, color_g, color_b));

    M5.Lcd.setCursor(0, 0);
    M5.Lcd.print("IP:");
#ifdef ACTS_AS_AP_MODE
    M5.Lcd.println(WiFi.softAPIP());
#else
    M5.Lcd.println(WiFi.localIP());
#endif    
    M5.Lcd.print("A:");
    M5.Lcd.println(value_a);
}

void loop() {
    irboard.update();

    M5.update();
    bool x0 = M5.BtnA.isPressed() || M5.BtnB.isPressed() || M5.BtnC.isPressed();

    irboard.setBoolValue("Y0", x0);
    value_a = irboard.shortValue("D0");

    color_r = irboard.shortValue("D1");
    color_g = irboard.shortValue("D2");
    color_b = irboard.shortValue("D3");

    if (irboard.isChanged()) {
        display_info();
    }
}
