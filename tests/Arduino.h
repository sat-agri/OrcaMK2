#pragma once

// Only the hardware boundary is simulated; checks compile the real firmware.
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <vector>

using byte = uint8_t;
using std::abs;
constexpr int LOW = 0, HIGH = 1, OUTPUT = 1;
constexpr byte D5 = 5, D6 = 6, A3 = 17, A4 = 18;
constexpr byte LEDR = 20, LEDG = 21, LEDB = 22;

extern unsigned long clockMs;
extern int digitalPins[64];
extern int pwmPins[64];

inline unsigned long millis() { return clockMs; }
inline void delay(unsigned long ms) { clockMs += ms; }
inline void pinMode(int, int) {}
inline void digitalWrite(int pin, int value) { digitalPins[pin] = value; }
inline void analogWrite(int pin, int value) { pwmPins[pin] = value; }
inline long map(long value, long inMin, long inMax, long outMin, long outMax) {
    return (value - inMin) * (outMax - outMin) / (inMax - inMin) + outMin;
}
template <typename T> T constrain(T value, T minimum, T maximum) {
    return std::max(minimum, std::min(value, maximum));
}

struct SerialPort {
    std::deque<uint8_t> incoming;
    std::vector<uint8_t> outgoing;
    void begin(unsigned long) {}
    int available() { return static_cast<int>(incoming.size()); }
    int read() {
        const int value = incoming.front();
        incoming.pop_front();
        return value;
    }
    size_t write(const uint8_t* data, size_t size) {
        outgoing.insert(outgoing.end(), data, data + size);
        return size;
    }
};
extern SerialPort Serial, Serial1;
