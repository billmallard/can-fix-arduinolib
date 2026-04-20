/*
 * Mock can.h for unit testing canfix.cpp outside of the Arduino environment.
 * Records last frame written so tests can inspect it.
 */
#pragma once
#include <cstdint>

typedef uint8_t  byte;
typedef uint16_t word;

struct CanFrame {
    uint16_t id;
    uint8_t  eid;
    uint8_t  data[8];
    uint8_t  length;
};

// Commands and modes used by canfix.cpp
#define CMD_RESET     0
#define MODE_CONFIG   1
#define MODE_NORMAL   2

// Recorded state for test inspection
struct MockCanState {
    CanFrame last_written;
    int      write_count;
    uint8_t  rx_status;       // returned by getRxStatus()
    CanFrame rx_frame[2];     // frame available for reading

    MockCanState() : write_count(0), rx_status(0) {
        last_written = {};
        rx_frame[0] = {};
        rx_frame[1] = {};
    }
};

extern MockCanState g_can;

class CAN {
public:
    explicit CAN(uint8_t pin) { (void)pin; }
    void sendCommand(uint8_t cmd) { (void)cmd; }
    void setBitRate(int rate) { (void)rate; }
    void setMode(uint8_t mode) { (void)mode; }

    uint8_t writeFrame(CanFrame frame) {
        g_can.last_written = frame;
        g_can.write_count++;
        return 0; // success
    }

    uint8_t getRxStatus() { return g_can.rx_status; }

    CanFrame readFrame(uint8_t buffer) {
        return g_can.rx_frame[buffer];
    }
};
