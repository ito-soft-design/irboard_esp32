// The MIT License (MIT)
//
// Copyright (c) 2020 ITO SOFT DESIGN Inc.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#ifndef irboard_h
#define irboard_h

#include <WiFi.h>
#include <WiFiMulti.h>
#include <WiFiAP.h>
#include <string>

#define IRBOARD_STATE_INITIAL       0
#define IRBOARD_STATE_CONNECTING    1
#define IRBOARD_STATE_LISTEN        2
#define IRBOARD_STATE_CONNECTED     3
#define IRBOARD_STATE_CLOSED        4

#ifndef IRBOARD_SIZE_X
#define IRBOARD_SIZE_X              32
#endif
#ifndef IRBOARD_SIZE_Y
#define IRBOARD_SIZE_Y              32
#endif
#ifndef IRBOARD_SIZE_M
#define IRBOARD_SIZE_M              32
#endif
#ifndef IRBOARD_SIZE_H
#define IRBOARD_SIZE_H              32
#endif
#ifndef IRBOARD_SIZE_D
#define IRBOARD_SIZE_D              128
#endif
#ifndef IRBOARD_SIZE_SD
#define IRBOARD_SIZE_SD             32
#endif


class Irboard {

    public:
        Irboard(int portNo = 5555);
        void begin(bool apMode = false);
        void terminate();
        void update();
        int state() { return _state; }
        int portNo() { return _portNo; }
        bool addAP(const char* ssid, const char *passphrase);
        void resetPermission();

        void setVerbose(bool verbose) { _verbose = verbose; }
        bool verbose() { return _verbose; }
        bool timeout() { return _timeout; }
        bool setTimeout(unsigned long timeout) { _timeout = timeout; }
 
        bool boolValue(std::string dev);
        void setBoolValue(std::string dev, bool value);
        short shortValue(std::string dev);
        void setShortValue(std::string dev, short value);
        int intValue(std::string dev);
        void setIntValue(std::string dev, int value);
        float floatValue(std::string dev);
        void setFloatValue(std::string dev, float value);
        String stringValue(std::string dev, int maxSize);
        void setStringValue(std::string dev, String value, int maxSize=0);

        bool isChanged() { return _changed; }

    private:
        int _state;
        int _portNo;
        bool _changed;
        bool _sd_dev;
        bool _verbose;
        bool _apMode;
        int _numOfAp;
        unsigned long _timeout;
        unsigned long _received_at;

        WiFiServer _server;
        WiFiClient _client;
        WiFiMulti _wifiMulti;
        std::string _recBuf;
        uint16_t devX[(IRBOARD_SIZE_X + 15) / 16];
        uint16_t devY[(IRBOARD_SIZE_Y + 15) / 16];
        uint16_t devM[(IRBOARD_SIZE_M + 15) / 16];
        uint16_t devH[(IRBOARD_SIZE_H + 15) / 16];
        uint16_t devD[IRBOARD_SIZE_D];
        uint16_t devSd[IRBOARD_SIZE_SD];
        
        void set_state(int state);
        void state_initial();
        void state_connecting();
        void state_listen();
        void state_connected();
        void state_closed();
        bool check_connection();
        uint8_t run();
        std::string response(std::string str);
        std::string rds_response(std::string opcode);
        std::string rd_response(std::string opcode);
        std::string wrs_response(std::string opcode);
        std::string wr_response(std::string opcode);
        std::string st_response(std::string opcode);
        std::string rs_response(std::string opcode);
        uint16_t *vptr_for_dev(std::string dev, int size, int *bit);
};

#endif
