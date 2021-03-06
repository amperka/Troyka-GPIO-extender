/*
 * This file is a part of Dragster car set library
 *
 * Realise: octoliner object - 8-point line-folowing sensor driver
 * © Amperka LLC (https://amperka.com, dev@amperka.com)
 * 
 * Author: Vasily Basalaev <vasily@amperka.ru>
 * Refactored by: Yury Botov <by@amperka.com>
 * License: GPLv3, all text here must be included in any redistribution.
 */

#include "Octoliner.h"

Octoliner::Octoliner(uint8_t i2caddress)
    : GpioExpander(i2caddress) {
    _value = 0.;
}

void Octoliner::begin() {
    begin(&Wire);
}

void Octoliner::begin(TwoWire* wire) {
    GpioExpander::begin(wire);
    GpioExpander::pwmFreq(8000); // ~ 250 different pwm values
}

void Octoliner::setSensitivity(uint8_t sense) {
    analogWrite(_sensePin, sense);
}

void Octoliner::setBrightness(uint8_t brightness) {
    analogWrite(_ledBrightnessPin, brightness);
}

int Octoliner::analogRead(uint8_t sensor) {
    sensor &= 0x07;
    return GpioExpander::analogRead(_sensorPinMap[sensor]);
}

uint8_t Octoliner::getBinaryLine() {
    uint8_t result = 0;
    int port = digitalReadPort();
    for (uint8_t i = 0; i < 8; ++i) {
        int mask = 1;
        mask <<= _sensorPinMap[i];
        if (port & mask)
            result |= 1 << i;
    }
    return result;
}

uint8_t Octoliner::getBinaryLine(uint16_t treshold) {
    uint8_t result = 0;
    for (uint8_t i = 0; i < 8; ++i) {
        int value = analogRead(i);
        if (treshold < value)
            result |= 1 << i;
    }
    return result;
}

float Octoliner::mapLine(uint8_t binaryLine) {
    int8_t sum = 0;
    int8_t avg = 0;
    int8_t weight[] = { 4, 3, 2, 1, -1, -2, -3, -4 };
    for (uint8_t i = 0; i < 8; i++) {
        bool isHigh = binaryLine & (1 << i);
        if (isHigh) {
            sum += 1;
            avg += weight[i];
        }
    }
    if (sum != 0) {
        return (float)avg / (float)sum / 4.0;
    }
    return 0;
}

float Octoliner::mapLine(int binaryLine[8]) {
    byte pattern = 0;
    // search min and max values
    int min = 32767;
    int max = 0;
    for (int i = 0; i < 8; i++) {
        if (binaryLine[i] < min)
            min = binaryLine[i];
        if (binaryLine[i] > max)
            max = binaryLine[i];
    }
    // calculate border level
    int threshold = min + (max - min) / 2;
    // create bit pattern
    for (int i = 0; i < 8; i++) {
        pattern = (pattern << 1) + ((binaryLine[i] < threshold) ? 0 : 1);
    }
    switch (pattern) {
    case 0b00011000:
        _value = 0;
        break;

    case 0b00010000:
        _value = 0.25;
        break;
    case 0b00111000:
        _value = 0.25;
        break;
    case 0b00001000:
        _value = -0.25;
        break;
    case 0b00011100:
        _value = -0.25;
        break;

    case 0b00110000:
        _value = 0.375;
        break;
    case 0b00001100:
        _value = -0.375;
        break;

    case 0b00100000:
        _value = 0.5;
        break;
    case 0b01110000:
        _value = 0.5;
        break;
    case 0b00000100:
        _value = -0.5;
        break;
    case 0b00001110:
        _value = -0.5;
        break;

    case 0b01100000:
        _value = 0.625;
        break;
    case 0b11100000:
        _value = 0.625;
        break;
    case 0b00000110:
        _value = -0.625;
        break;
    case 0b00000111:
        _value = -0.625;
        break;

    case 0b01000000:
        _value = 0.75;
        break;
    case 0b11110000:
        _value = 0.75;
        break;
    case 0b00000010:
        _value = -0.75;
        break;
    case 0b00001111:
        _value = -0.75;
        break;

    case 0b11000000:
        _value = 0.875;
        break;
    case 0b00000011:
        _value = -0.875;
        break;

    case 0b10000000:
        _value = 1.0;
        break;
    case 0b00000001:
        _value = -1.0;
        break;

    default:
        break; // for other patterns return previous value
    }
    return _value;
}
