
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

#include "irboard.h"
#include <sstream>

// #define IRBOARD_DEBUG

Irboard::Irboard(int portno)
{
    _portNo = portno;
    _state = IRBOARD_STATE_INITIAL;
#ifdef IRBOARD_DEBUG
    Serial.println("initial");
#endif
    // clear the device values
    memset(devX, 0, sizeof(uint16_t) * IRBOARD_SIZE_X);
    memset(devY, 0, sizeof(uint16_t) * IRBOARD_SIZE_Y);
    memset(devM, 0, sizeof(uint16_t) * IRBOARD_SIZE_M);
    memset(devH, 0, sizeof(uint16_t) * IRBOARD_SIZE_H);
    memset(devD, 0, sizeof(uint16_t) * IRBOARD_SIZE_D);
    memset(devSd, 0, sizeof(uint16_t) * IRBOARD_SIZE_SD);
}

bool Irboard::addAP(const char* ssid, const char *passphrase)
{
  return _wifiMulti.addAP(ssid, passphrase);
}

void Irboard::begin(bool apMode)
{
    terminate();
    _apMode = apMode;
    _recBuf = "";
}

void Irboard::update()
{
    _changed = false;
    switch(_state) {
    case IRBOARD_STATE_INITIAL:
        state_initial();
        break;
    case IRBOARD_STATE_CONNECTING:
        state_connecting();
        break;
    case IRBOARD_STATE_LISTEN:
        state_listen();
        break;
    case IRBOARD_STATE_CONNECTED:
        state_connected();
        break;
    case IRBOARD_STATE_CLOSED:
        state_closed();
        break;
    }
}

void Irboard::resetPermission()
{
    uint16_t *ptr = devSd;
    for(int i = 0; i < IRBOARD_SIZE_SD; i++) {
        *ptr++ = 0;
    }
}

void Irboard::set_state(int state)
{
    _state = state;
    _changed = true;
}

void Irboard::state_initial()
{
    set_state(IRBOARD_STATE_CONNECTING);
#ifdef IRBOARD_DEBUG
    Serial.println("connecting");
#endif
}

void Irboard::state_connecting()
{
    if (_apMode || _wifiMulti.run() == WL_CONNECTED) {
        if (_server == false) {
            _server = WiFiServer(_portNo, 1);
            _server.begin();
        }
        set_state(IRBOARD_STATE_LISTEN);
#ifdef IRBOARD_DEBUG
        Serial.println("listen");
#endif
        if (_verbose) {
            if (_apMode) {
                Serial.print(WiFi.softAPIP());
            } else {
                Serial.println(WiFi.SSID());
                Serial.println(WiFi.localIP());
            }
        }
    }
}

bool Irboard::check_connection()
{
    if (_apMode) return true;
    if (_wifiMulti.run() != WL_CONNECTED) {
        terminate();
        return false;
    }
    return true;
}

void Irboard::terminate()
{
    set_state(IRBOARD_STATE_INITIAL);
    _changed = false;
#ifdef IRBOARD_DEBUG
    Serial.println("connecting");
#endif
    if (_client) {
        _client.stop();
        _client.~WiFiClient();
    }
    if (_server) {
        _server.stopAll();
        _server.~WiFiServer();
    }
    WiFi.disconnect();
}

void Irboard::state_listen()
{
    if (check_connection() == false) { return; }

    _client = _server.available();
    if (_client) {
        set_state(IRBOARD_STATE_CONNECTED);
#ifdef IRBOARD_DEBUG
        Serial.println("connected");
#endif
    }
}

void Irboard::state_connected()
{
    if (check_connection() == false) { return; }

    if (_client.connected()) {
        if (_client.available()) {
            char c = _client.read();
#ifdef IRBOARD_DEBUG
            Serial.write(c);
#endif
            _recBuf += c;
            if (c == 0x0a) {
                std::string r = response(_recBuf);
                _client.print(r.c_str());
                _client.print("\r\n");
                _client.flush();
#ifdef IRBOARD_DEBUG
                Serial.println(r.c_str());
#endif
                _recBuf = "";
            }
        }
    } else {
        set_state(IRBOARD_STATE_CLOSED);
#ifdef IRBOARD_DEBUG
        Serial.println("closed");
#endif
    }
}

void Irboard::state_closed()
{
    _client.stop();
    set_state(IRBOARD_STATE_CONNECTING);
#ifdef IRBOARD_DEBUG
    Serial.println("connecting");
#endif
}

std::string Irboard::response(std::string str)
{
    int p = str.find_first_of(" ");
    if (p == std::string::npos) { return "E1"; }

    std::string cmd = str.substr(0, p);
    std::string opcode = str.substr(p + 1);
    if (cmd == "RDS") {
        return rds_response(opcode);
    } else if (cmd == "RD") {
        return rd_response(opcode);
    } else if (cmd == "WRS") {
        return wrs_response(opcode);
    } else if (cmd == "WR") {
        return wr_response(opcode);
    } else if (cmd == "ST") {
        return st_response(opcode);
    } else if (cmd == "RS") {
        return rs_response(opcode);
    }
    return "E1";
}


std::string Irboard::rds_response(std::string opcode)
{
    int p = opcode.find_first_of(" ");
    if (p == std::string::npos) { return "E1"; }
    std::string dev = opcode.substr(0, p);
    std::string str = opcode.substr(p + 1);

    int vBase = 10;
    p = dev.find_first_of(".");
    if (p != std::string::npos) {
        if (dev.substr(p + 1, 1) == "H") {
            vBase = 16;
        }
        dev = dev.substr(0, p);
    }

    int num = atoi(str.c_str());

    std::string r = "";

    uint16_t *ptr = vptr_for_dev(dev, num);
    if (ptr == NULL) { return "E0"; }

    if (vBase == 10) {
        for (int i = 0; i < num; i++) {
            if (i != 0) {
                r += " ";
            }
            std::stringstream ss;
            ss << std::dec << *ptr++;
            r += ss.str();
        }
    } else {
        for (int i = 0; i < num; i++) {
            if (i != 0) {
                r += " ";
            }
            //char buf[5] = { '\0' };
            //sprintf(buf, "%04X", *ptr++);
            //r += std::string(buf);
            std::stringstream ss;
            ss << std::uppercase << std::hex << *ptr++;
            r += ss.str();
        }
    }
    return r;
}

std::string Irboard::rd_response(std::string opcode)
{
    std::string dev = opcode;

    int vBase = 10;
    int p = dev.find_first_of(".");
    if (p != std::string::npos) {
        if (dev.substr(p + 1, 1) == "H") {
            vBase = 16;
        }
        dev = dev.substr(0, p);
    }

    std::string r = "";

    uint16_t *ptr = vptr_for_dev(dev, 1);
    if (ptr == NULL) { return "E0"; }

    if (vBase == 10) {
        std::stringstream ss;
        ss << std::dec << *ptr++;
        r += ss.str();
    } else {
        std::stringstream ss;
        ss << std::uppercase << std::hex << *ptr++;
        r += ss.str();
    }
    return r;
}

std::string Irboard::wrs_response(std::string opcode)
{
    int p = opcode.find_first_of(" ");
    if (p == std::string::npos) { return "E1"; }
    std::string dev = opcode.substr(0, p);
    std::string str = opcode.substr(p + 1);
    int vBase = 10;
    p = dev.find_first_of(".");
    if (p != std::string::npos) {
        if (dev.substr(p + 1, 1) == "H") {
            vBase = 16;
        }
        dev = dev.substr(0, p);
    }

    p = str.find_first_of(" ");
    if (p == std::string::npos) { return "E1"; }
    int num = atoi(str.substr(0, p).c_str());

    str = str.substr(p + 1);

    uint16_t *ptr = vptr_for_dev(dev, num);
    if (ptr == NULL) { return "E0"; }

    for (int i = 0; i < num - 1; i++) {
        p = str.find_first_of(" ");
        if (p == std::string::npos) { return "E1"; }
        *ptr++ = (uint16_t)strtol(str.substr(0, p).c_str(), NULL, vBase);
        str = str.substr(p + 1);
    }
    *ptr++ = (uint16_t)strtol(str.substr(0, p).c_str(), NULL, vBase);

    if (_sd_dev == false) _changed = true;
    return "OK";
}

std::string Irboard::wr_response(std::string opcode)
{
    int p = opcode.find_first_of(" ");
    if (p == std::string::npos) { return "E1"; }
    std::string dev = opcode.substr(0, p);
    std::string str = opcode.substr(p + 1);
    
    int vBase = 10;
    p = dev.find_first_of(".");
    if (p != std::string::npos) {
        if (dev.substr(p + 1, 1) == "H") {
            vBase = 16;
        }
        dev = dev.substr(0, p);
    }

    uint16_t *ptr = vptr_for_dev(dev, 1);
    if (ptr == NULL) { return "E0"; }

    *ptr = (uint16_t)strtol(str.c_str(), NULL, vBase);

    if (_sd_dev == false) _changed = true;
    return "OK";
}

std::string Irboard::st_response(std::string opcode)
{
    std::string dev = opcode;
    uint16_t *ptr = vptr_for_dev(dev, 1);
    if (ptr == NULL) { return "E0"; }

    *ptr = 1;
    
    if (_sd_dev == false) _changed = true;
    return "OK";
}

std::string Irboard::rs_response(std::string opcode)
{
    std::string dev = opcode;
    uint16_t *ptr = vptr_for_dev(dev, 1);
    if (ptr == NULL) { return "E0"; }

    *ptr = 0;
    
    if (_sd_dev == false) _changed = true;
    return "OK";
}

bool Irboard::boolValue(std::string dev)
{
    uint16_t *ptr = vptr_for_dev(dev, 1);
    return ptr ? *ptr != 0 : false;
}

void Irboard::setBoolValue(std::string dev, bool value)
{
    uint16_t *ptr = vptr_for_dev(dev, 1);
    if (ptr) {
        *ptr = value ? 1 : 0;
    }
}

short Irboard::shortValue(std::string dev)
{
    uint16_t *ptr = vptr_for_dev(dev, 1);
    return ptr ? *ptr : 0;
}

void Irboard::setShortValue(std::string dev, short value)
{
    uint16_t *ptr = vptr_for_dev(dev, 1);
    if (ptr) {
        *ptr = value;
    }
}

int Irboard::intValue(std::string dev)
{
    unsigned int v = 0;
    uint16_t *ptr = vptr_for_dev(dev, 2);
    if (ptr) {
        v = *(unsigned int *)ptr;
        // DELETEME: v |= (unsigned int)(*ptr << 16);
    }
    return ptr ? (int)v : 0;
}

void Irboard::setIntValue(std::string dev, int value)
{
    uint16_t *ptr = vptr_for_dev(dev, 1);
    if (ptr) {
        *ptr++ = value;
        *ptr = value >> 16;
    }
}

float Irboard::floatValue(std::string dev)
{
    int value = intValue(dev);
    return *((float *)&value);
}

void Irboard::setFloatValue(std::string dev, float value)
{
    setIntValue(dev, *((int *)&value));
}

String Irboard::stringValue(std::string dev, int maxSize)
{
    int size = (maxSize + 1) / 2;
    uint16_t *ptr = vptr_for_dev(dev, size);
    String str = String();
    if (ptr) {
        for (int i = 0; i < size; i++) {
            char l = *ptr & 0xff;
            char h = *ptr >> 8;
            if (l != '\0') {
                str += l;
            } else {
                break;
            }
            if (h != '\0') {
                str += h;
            } else {
                break;
            }
            ptr++;
        }
    }
    return str;
}

void Irboard::setStringValue(std::string dev, String value, int maxSize)
{
    int len = value.length();
    int size = (min(len, maxSize == 0 ? len : maxSize) + 1) / 2;
    uint16_t *ptr = vptr_for_dev(dev, size);
    if (ptr) {
        uint16_t word_value = 0;
        const char *str_ptr = value.c_str();
        char ch;
        for (int i = 0; i < size; i++) {
            int idx = i * 2;
            ch = idx < len ? str_ptr[idx] : 0;
            word_value = ch;
            idx++;
            ch = idx < len ? str_ptr[idx] : 0;
            word_value |= (uint16_t)ch << 8;
            *ptr++ = word_value;
        }
    }
}


uint16_t *Irboard::vptr_for_dev(std::string dev, int size)
{
    _sd_dev = false;
    std::string s2 = dev.substr(0, 2);
    int no2 = atoi(dev.substr(2).c_str());
    if (s2 == "SD") {
        _sd_dev = true;
        if (no2 + size > IRBOARD_SIZE_SD) return NULL;
        return &(devSd[no2]);
    }

    std::string s1 = dev.substr(0, 1);
    int no1 = atoi(dev.substr(1).c_str());
    if (s1 == "X") {
        if (no1 + size > IRBOARD_SIZE_X) return NULL;
        return &(devX[no1]);
    } else if (s1 == "Y") {
        if (no1 + size > IRBOARD_SIZE_Y) return NULL;
        return &(devY[no1]);
    } else if (s1 == "M") {
        if (no1 + size > IRBOARD_SIZE_M) return NULL;
        return &(devM[no1]);
    } else if (s1 == "H") {
        if (no1 + size > IRBOARD_SIZE_H) return NULL;
        return &(devH[no1]);
    } else if (s1 == "D") {
        if (no1 + size > IRBOARD_SIZE_D) return NULL;
        return &(devD[no1]);
    }

    return NULL;
}
