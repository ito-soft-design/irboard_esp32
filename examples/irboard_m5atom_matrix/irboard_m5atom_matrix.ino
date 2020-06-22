#include <M5Atom.h>
#include <irboard.h>

Irboard irboard = Irboard();

const char *ssid = "ssid";
const char *password = "password";

uint8_t DisBuff[2 + 5 * 5 * 3];

void setBuff(uint8_t Rdata, uint8_t Gdata, uint8_t Bdata)
{
    DisBuff[0] = 0x05;
    DisBuff[1] = 0x05;
    for (int i = 0; i < 25; i++)
    {
        DisBuff[2 + i * 3 + 0] = Rdata;
        DisBuff[2 + i * 3 + 1] = Gdata;
        DisBuff[2 + i * 3 + 2] = Bdata;
    }
}

void displayLeds() {
    short r = irboard.shortValue("D1");
    short g = irboard.shortValue("D2");
    short b = irboard.shortValue("D3");
    setBuff(r, g, b);
    M5.dis.displaybuff(DisBuff);    
}

void setup() {
    M5.begin(true, false, true);
    Serial.begin(115200);

    // set wifi ssid & password
    irboard.addAP(ssid, password);
    // show ip address to terminal
    irboard.setVerbose(true);
    // 
    irboard.begin();

    // set default color
    irboard.setShortValue("D1", 0x80);      // Red
    irboard.setShortValue("D2", 0);         // Green
    irboard.setShortValue("D3", 0);         // Blue

    delay(10);
    displayLeds();
}

void loop() {
    irboard.update();
    if (irboard.isChanged()) {
        displayLeds();
    }

    M5.update();
    irboard.setBoolValue("Y0", M5.Btn.isPressed());
}