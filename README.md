# irBoard Library for ESP32

[(To Japanese)](./README_ja.md)

irBoard Library for ESP32 is a library for Arduino IDE to connect to ESP32 devices from the iOS app irBoard and to be able to control, monitor, and configure the devices.

irBoard is an iOS application that allows iPods and iPhones to be used as touch panel displays for [PLC(Programable logic controller)](https://ja.wikipedia.org/wiki/プログラマブルロジックコントローラ).

[irBoard](https://irboard.itosoft.com)

This library lets you control, monitor, and configure devices from your iPad or iPhone.

## Tutorial

Please see the Qiit article on importing the library and connecting it to irBoard.

[We have created a library that can connect from irBoard to M5StickC and other devices. - Qiita - Japanese](https://qiita.com/katsuyoshi/items/5c850cd14e8e2ff47b4f)

# Usage

1. Include irboard.h
2. Create an instance of Irboard
3. Set WiFi SSID and password with addAP().  
  Since WiFiMulti is used, you can register multiple access points by repeating addAP().
4. pass "true" to setVerbose() to output the IP address assigned at the time of connection to the Terminal.
5. call begin() to start a connection with irBoard.

```
#include <irboard.h>                    // 1

Irboard irboard = Irboard();            // 2

void setup() {
    .
    .
    irboard.addAP(ssid, password);      // 3
    irboard.setVerbose(true);           // 4
    irboard.begin();                    // 5
}
```

irBoard can be connected to PLCs for operation and monitoring.
Instead of PLCs, there is the Ruby gem Ladder Drive, which allows a PC or Raspberry PI to be used as a virtual PLC, and irBoard can be connected to this as well.

[GitHub - ito-soft-design/ladder_drive](https://github.com/ito-soft-design/ladder_drive)

The irBoard Library for ESP32 is designed to be regarded as having a Ladder Drive virtual PLC device.

This library can handle the following PLC devices.

|device(range)|type|use|
|:--|:-:|:--|
|X00 - X31|bool|Input devices such as switches|
|Y00 - Y31|bool|Output devices such as lamps|
|M00 - M31|bool|Auxiliary coils|
|H00 - H31|bool|retention coil. It is originally memorized when the power is turned off, but it is not memorized|
|D00 - D127|short|data memory (16bit)|
|SD00 - SD32|short|used for interaction with irBoard|

The device size is defined in [irboard.h](https://github.com/ito-soft-design/irboard_esp32/blob/master/src/irboard.h#L37)  
It can be changed by redefining it in the compile options or by changing this file directly.

Since it is not a PLC, there is no device like this. The image is that there is a Hash with the device name as a key, and it reads and writes to it.  

7. update() is used to send/receive data to/from irBoard and update the device value. 
8. isChanged() can be used to check if the value has changed and update the screen, etc. when there is a change. 
9. When reading values, call boolValue(), shortValue(), intValue(), or floatValue() with the device name, depending on the device type. intValue and floatValue use two words. If D0 is specified, D0 and the next D1 are connected and treated as 32 bits.
10. setBoolValue(,), setShortValue(,), setIntValue(,), and setFloatValue(,) are used for writing.


```
bool x0;
short d0;

void loop() {
    irboard.update();                           // 7
    if (irboard.isChanged()) {                  // 8
        x0 = irboard.boolValue("X0");           // 9
        d0 = irboard.shortValue("D0");          // 9

        // Updates, etc.
    }
    short temprature = getTemperatur(); // Assuming we have a function to get the temperature
    irboard.setShortValue("D1", temprature);        // 10
    irboard.setBoolValue("Y0", M5.Btn.isPressed()); // 10
}

```

## Examples

- Arduino IDE  
  Change the color of the screen and LEDs on the M5 device from irBoard, or indicate that a button has been pressed on the M5 device.
    - [for M5Atom](examples/irboard_m5atom)
    - [for M5Stack Basic](examples/irboard_m5stack)
    - [for M5Stick-C](examples/irboard_m5stickc)
- PlatformIO
    - [Basic (It's the same as Arduino IDE)](examples/platformio_irboard_basic)
    - [Control ATOM Mate for toio from irBoard](examples/platformio_irboard_toio_mate)
    - [WiFi setup with irBoard](examples/platformio_irboard_wifi_setup)

## About irBoard

For more information about irBoard, please visit the official website and documentation.  

[irBoard](https://irboard.itosoft.com/en/)

[Document (Sorry we only have Japanese documents.)](https://irboard.itosoft.com/ja/document/document)

### LICENSE

[MIT](https://github.com/ito-soft-design/irboard_esp32/blob/master/LICENSE.txt)
