#include "display.h"
#include "state_machine.h"
#include "config.h"

#include <cstdio>
#include <cstring>

// ===========================================================================
// ESP32 implementation
// ===========================================================================
#ifndef NATIVE_TEST

#include <Wire.h>
#include <U8g2lib.h>

// ---------------------------------------------------------------------------
// OLED display (SH1107 128x128, I2C)
// ---------------------------------------------------------------------------

class OledDisplay : public DisplayInterface {
public:
    OledDisplay()
        : u8g2_(U8G2_R0, /* reset */ U8X8_PIN_NONE) {}

    void init() override {
        u8g2_.begin();
        u8g2_.setFont(u8g2_font_6x10_tr);
        u8g2_.clearBuffer();
        u8g2_.sendBuffer();
    }

    void showBoot(const char* version) override {
        u8g2_.clearBuffer();
        u8g2_.setFont(u8g2_font_7x14B_tr);
        u8g2_.drawStr(10, 40, "Slider");
        u8g2_.setFont(u8g2_font_6x10_tr);
        u8g2_.drawStr(10, 60, version);
        u8g2_.drawStr(10, 80, "Booting...");
        u8g2_.sendBuffer();
    }

    void showStatus(const char* state, const char* extra) override {
        u8g2_.clearBuffer();
        u8g2_.setFont(u8g2_font_7x14B_tr);
        u8g2_.drawStr(0, 14, "State:");
        u8g2_.drawStr(50, 14, state);

        if (extra) {
            u8g2_.setFont(u8g2_font_6x10_tr);
            u8g2_.drawStr(0, 30, extra);
        }
        u8g2_.sendBuffer();
    }

    void showPlayback(uint32_t elapsed_ms, uint32_t total_ms,
                      const float* positions, uint8_t axis_count) override {
        u8g2_.clearBuffer();
        u8g2_.setFont(u8g2_font_6x10_tr);

        // Progress bar
        char line[32];
        uint32_t elapsed_s = elapsed_ms / 1000;
        uint32_t total_s   = total_ms / 1000;
        snprintf(line, sizeof(line), "%lu/%lus",
                 (unsigned long)elapsed_s, (unsigned long)total_s);
        u8g2_.drawStr(0, 10, line);

        if (total_ms > 0) {
            uint16_t bar_w = static_cast<uint16_t>(
                (elapsed_ms * 120UL) / total_ms);
            if (bar_w > 120) bar_w = 120;
            u8g2_.drawFrame(0, 14, 124, 8);
            u8g2_.drawBox(2, 16, bar_w, 4);
        }

        // Axis positions
        uint8_t y = 34;
        for (uint8_t a = 0; a < axis_count && a < MAX_AXES; a++) {
            snprintf(line, sizeof(line), "%s: %.1f",
                     g_axis_config[a].name, positions[a]);
            u8g2_.drawStr(0, y, line);
            y += 12;
        }

        u8g2_.sendBuffer();
    }

    void showPositions(const float* positions, uint8_t axis_count,
                       const char* const* axis_names) override {
        u8g2_.clearBuffer();
        u8g2_.setFont(u8g2_font_6x10_tr);

        uint8_t y = 12;
        char line[32];
        for (uint8_t a = 0; a < axis_count && a < MAX_AXES; a++) {
            snprintf(line, sizeof(line), "%s: %.2f %s",
                     axis_names[a], positions[a], g_axis_config[a].unit);
            u8g2_.drawStr(0, y, line);
            y += 14;
        }

        u8g2_.sendBuffer();
    }

    void showError(const char* error_name) override {
        u8g2_.clearBuffer();
        u8g2_.setFont(u8g2_font_7x14B_tr);
        u8g2_.drawStr(0, 20, "ERROR");
        u8g2_.setFont(u8g2_font_6x10_tr);
        u8g2_.drawStr(0, 40, error_name);
        u8g2_.sendBuffer();
    }

    void showHoming(uint8_t axis_index, const char* axis_name) override {
        u8g2_.clearBuffer();
        u8g2_.setFont(u8g2_font_7x14B_tr);
        u8g2_.drawStr(0, 30, "Homing...");
        u8g2_.setFont(u8g2_font_6x10_tr);
        u8g2_.drawStr(0, 50, axis_name);
        u8g2_.sendBuffer();
    }

    void clear() override {
        u8g2_.clearBuffer();
        u8g2_.sendBuffer();
    }

private:
    U8G2_SH1107_128X128_F_HW_I2C u8g2_;
};

// ---------------------------------------------------------------------------
// Factory: I2C scan for display
// ---------------------------------------------------------------------------

DisplayInterface* createDisplay() {
    Wire.begin(PIN_SDA, PIN_SCL);
    Wire.beginTransmission(0x3C);
    uint8_t err = Wire.endTransmission();

    if (err == 0) {
        auto* disp = new OledDisplay();
        disp->init();
        return disp;
    }

    return new NullDisplay();
}

// ---------------------------------------------------------------------------
// FreeRTOS display update task (5 Hz)
// ---------------------------------------------------------------------------

void displayUpdateTask(void* param) {
    DisplayInterface* disp = static_cast<DisplayInterface*>(param);
    const TickType_t interval = pdMS_TO_TICKS(200);  // 5 Hz

    for (;;) {
        State     st  = g_state.current_state;
        ErrorCode ec  = g_state.error_code;

        switch (st) {
            case State::ERROR:
                disp->showError(errorCodeName(ec));
                break;

            case State::HOMING:
                disp->showHoming(AXIS_SLIDE, g_axis_config[AXIS_SLIDE].name);
                break;

            case State::PLAYING: {
                float pos[MAX_AXES];
                for (uint8_t a = 0; a < g_axis_count; a++)
                    pos[a] = g_state.positions[a];
                disp->showPlayback(g_state.trajectory_elapsed_ms,
                                   g_state.trajectory_duration_ms,
                                   pos, g_axis_count);
                break;
            }

            case State::PAUSED: {
                float pos[MAX_AXES];
                for (uint8_t a = 0; a < g_axis_count; a++)
                    pos[a] = g_state.positions[a];
                disp->showPlayback(g_state.trajectory_elapsed_ms,
                                   g_state.trajectory_duration_ms,
                                   pos, g_axis_count);
                break;
            }

            default: {
                float pos[MAX_AXES];
                const char* names[MAX_AXES];
                for (uint8_t a = 0; a < g_axis_count; a++) {
                    pos[a]   = g_state.positions[a];
                    names[a] = g_axis_config[a].name;
                }
                disp->showPositions(pos, g_axis_count, names);
                break;
            }
        }

        vTaskDelay(interval);
    }
}

// ===========================================================================
// NATIVE_TEST: NullDisplay only
// ===========================================================================
#else

DisplayInterface* createDisplay() {
    return new NullDisplay();
}

#endif // NATIVE_TEST
