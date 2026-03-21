#pragma once

#include <cstdint>

#ifndef MAX_AXES
#define MAX_AXES 4
#endif

// ---------------------------------------------------------------------------
// Abstract display interface
// ---------------------------------------------------------------------------

class DisplayInterface {
public:
    virtual ~DisplayInterface() = default;

    virtual void init() = 0;
    virtual void showBoot(const char* version) = 0;
    virtual void showStatus(const char* state, const char* extra = nullptr) = 0;
    virtual void showPlayback(uint32_t elapsed_ms, uint32_t total_ms,
                              const float* positions, uint8_t axis_count) = 0;
    virtual void showPositions(const float* positions, uint8_t axis_count,
                               const char* const* axis_names) = 0;
    virtual void showError(const char* error_name) = 0;
    virtual void showHoming(uint8_t axis_index, const char* axis_name) = 0;
    virtual void clear() = 0;
};

// ---------------------------------------------------------------------------
// Null display (no-op, used when no hardware display is detected)
// ---------------------------------------------------------------------------

class NullDisplay : public DisplayInterface {
public:
    void init() override {}
    void showBoot(const char*) override {}
    void showStatus(const char*, const char*) override {}
    void showPlayback(uint32_t, uint32_t, const float*, uint8_t) override {}
    void showPositions(const float*, uint8_t, const char* const*) override {}
    void showError(const char*) override {}
    void showHoming(uint8_t, const char*) override {}
    void clear() override {}
};

// ---------------------------------------------------------------------------
// Factory & task
// ---------------------------------------------------------------------------

/// Probe I2C for a display at 0x3C. Returns an OledDisplay if found,
/// otherwise a NullDisplay.
DisplayInterface* createDisplay();

#ifndef NATIVE_TEST
/// FreeRTOS task: reads shared state and updates the display at 5 Hz.
void displayUpdateTask(void* param);
#endif
