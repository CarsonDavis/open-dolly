#pragma once

#include <cstdint>

#ifndef NATIVE_TEST
#include <Arduino.h>

// Stepper pins
#define PIN_STEP        GPIO_NUM_15
#define PIN_DIR         GPIO_NUM_16
#define PIN_EN          GPIO_NUM_17

// TMC2209 UART
#define PIN_TMC_UART_TX GPIO_NUM_18
#define PIN_TMC_UART_RX GPIO_NUM_18

// CAN bus (TWAI)
#define PIN_CAN_TX      GPIO_NUM_5
#define PIN_CAN_RX      GPIO_NUM_6

// Limit switches (active LOW with internal pullup)
#define PIN_LIMIT_MIN   GPIO_NUM_7
#define PIN_LIMIT_MAX   GPIO_NUM_8

// I2C
#define PIN_SDA         GPIO_NUM_1
#define PIN_SCL         GPIO_NUM_2

// Battery ADC
#define PIN_VBAT        GPIO_NUM_4

// Status LED
#define PIN_STATUS_LED  GPIO_NUM_48

#endif // NATIVE_TEST

// ---------------------------------------------------------------------------
// Axis configuration
// ---------------------------------------------------------------------------

enum class AxisType : uint8_t {
    STEPPER,
    DJI_CAN,
    SERVO
};

struct AxisConfig {
    const char* name;
    AxisType    type;
    float       min;
    float       max;
    const char* unit;
    bool        can_home;
};

// Build-profile axis tables --------------------------------------------------

#if defined(BUILD_SLIDER_ONLY)

// Single-axis slider
static constexpr uint8_t AXIS_COUNT = 1;

static constexpr uint8_t AXIS_SLIDE = 0;

static const AxisConfig AXIS_CONFIGS[AXIS_COUNT] = {
    { "slide", AxisType::STEPPER, 0.0f, 1000.0f, "mm", true },
};

#elif defined(BUILD_SLIDER_SERVO)

// Slider + servo pan/tilt
static constexpr uint8_t AXIS_COUNT = 3;

static constexpr uint8_t AXIS_SLIDE = 0;
static constexpr uint8_t AXIS_PAN   = 1;
static constexpr uint8_t AXIS_TILT  = 2;

static const AxisConfig AXIS_CONFIGS[AXIS_COUNT] = {
    { "slide", AxisType::STEPPER, 0.0f,    1000.0f, "mm",  true  },
    { "pan",   AxisType::SERVO,   -180.0f, 180.0f,  "deg", false },
    { "tilt",  AxisType::SERVO,   -90.0f,  90.0f,   "deg", false },
};

#else

// Default: slider + DJI gimbal (pan/tilt/roll)
static constexpr uint8_t AXIS_COUNT = 4;

static constexpr uint8_t AXIS_SLIDE = 0;
static constexpr uint8_t AXIS_PAN   = 1;
static constexpr uint8_t AXIS_TILT  = 2;
static constexpr uint8_t AXIS_ROLL  = 3;

static const AxisConfig AXIS_CONFIGS[AXIS_COUNT] = {
    { "slide", AxisType::STEPPER, 0.0f,    1000.0f, "mm",  true  },
    { "pan",   AxisType::DJI_CAN, -180.0f, 180.0f,  "deg", false },
    { "tilt",  AxisType::DJI_CAN, -90.0f,  45.0f,   "deg", false },
    { "roll",  AxisType::DJI_CAN, -45.0f,  45.0f,   "deg", false },
};

#endif

// Runtime-accessible copies (defined in config.cpp)
extern const AxisConfig* g_axis_config;
extern uint8_t           g_axis_count;
