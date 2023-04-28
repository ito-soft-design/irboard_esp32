#include <M5Atom.h>
#include <irboard.h>

// If you want to act as an ap mode, uncomment here.
// #define ACTS_AS_AP_MODE

Irboard irboard = Irboard();

// Set your ssid and password here.
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

    // set default color
    irboard.setShortValue("D1", 0x80);      // Red
    irboard.setShortValue("D2", 0);         // Green
    irboard.setShortValue("D3", 0);         // Blue

    delay(10);
    displayLeds();
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
        displayLeds();
    }

    M5.update();
    irboard.setBoolValue("Y0", M5.Btn.isPressed());
}