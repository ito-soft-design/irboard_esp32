#include <M5StickC.h>
#include <irboard.h>

const char *ssid = "ssid";
const charb*password = "password";

Irboard irboard = Irboard();

short value_a = 0;
short color_r = 0;
short color_g = 0;
short color_b = 0;

void setup() {
    M5.begin();
    Serial.begin(115200);

    M5.Axp.ScreenBreath(10);
    M5.Lcd.setRotation(3);
    M5.Lcd.setTextSize(2);
    M5.Lcd.fillScreen(BLACK);

    pinMode(GPIO_NUM_10, OUTPUT);
    digitalWrite(GPIO_NUM_10, HIGH);

    irboard.addAP(ssid, password);
    irboard.begin();
}

void display_info()
{
    uint16_t c = M5.Lcd.color565(color_r, color_g, color_b);
    M5.Lcd.fillScreen(c);
    M5.Lcd.setCursor(0, 0, 1);

    M5.Lcd.print("IP:");
    M5.Lcd.println(WiFi.localIP());
    
    M5.Lcd.print("A:");
    M5.Lcd.println(value_a);
}

void loop() {
    irboard.update();

    bool x0 = irboard.boolValue("X0");
    digitalWrite(GPIO_NUM_10, x0 ? LOW : HIGH);

    irboard.setBoolValue("Y0", x0);
    value_a = irboard.shortValue("D0");

    color_r = irboard.shortValue("D1");
    color_g = irboard.shortValue("D2");
    color_b = irboard.shortValue("D3");

    if (irboard.isChanged()) {
        display_info();
    }
}
