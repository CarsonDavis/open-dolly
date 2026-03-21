#include "stepper.h"
#include "config.h"
#include "state_machine.h"

volatile bool g_limit_hit = false;

// ===========================================================================
// ESP32 implementation
// ===========================================================================
#ifndef NATIVE_TEST

#include <Arduino.h>
#include <FastAccelStepper.h>

static FastAccelStepperEngine engine;
static FastAccelStepper* stepper = nullptr;

// Homing parameters
static constexpr float HOMING_SPEED_FAST_MM_S  = 20.0f;  // initial approach
static constexpr float HOMING_SPEED_SLOW_MM_S  = 2.0f;   // back-off re-approach
static constexpr float HOMING_BACKOFF_MM       = 5.0f;    // retreat distance

// ---------------------------------------------------------------------------
// Conversion
// ---------------------------------------------------------------------------

int32_t mmToSteps(float mm) {
    return static_cast<int32_t>(mm * STEPS_PER_MM);
}

float stepsToMm(int32_t steps) {
    return static_cast<float>(steps) * MM_PER_STEP;
}

// ---------------------------------------------------------------------------
// Limit switch ISR
// ---------------------------------------------------------------------------

void IRAM_ATTR onLimitSwitch() {
    g_limit_hit = true;
    // Immediately stop the stepper from the ISR context
    if (stepper) {
        stepper->forceStop();
    }
}

// ---------------------------------------------------------------------------
// Init
// ---------------------------------------------------------------------------

void stepperInit() {
    // Enable pin
    pinMode(PIN_EN, OUTPUT);
    stepperDisable();

    // Limit switches — active LOW with internal pullup
    pinMode(PIN_LIMIT_MIN, INPUT_PULLUP);
    pinMode(PIN_LIMIT_MAX, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(PIN_LIMIT_MIN), onLimitSwitch, FALLING);
    attachInterrupt(digitalPinToInterrupt(PIN_LIMIT_MAX), onLimitSwitch, FALLING);

    // FastAccelStepper engine
    engine.init();
    stepper = engine.stepperConnectToPin(PIN_STEP);
    if (stepper) {
        stepper->setDirectionPin(PIN_DIR);
        stepper->setEnablePin(PIN_EN);
        stepper->setAutoEnable(false);
        stepper->setSpeedInHz(static_cast<uint32_t>(HOMING_SPEED_FAST_MM_S * STEPS_PER_MM));
        stepper->setAcceleration(static_cast<uint32_t>(500.0f * STEPS_PER_MM));  // 500 mm/s^2
    }
}

void stepperEnable() {
    if (stepper) stepper->enableOutputs();
}

void stepperDisable() {
    if (stepper) stepper->disableOutputs();
}

void setStepperTarget(float position_mm) {
    if (!stepper) return;
    int32_t target = mmToSteps(position_mm);
    stepper->moveTo(target);
}

float getStepperPositionMm() {
    if (!stepper) return 0.0f;
    return stepsToMm(stepper->getCurrentPosition());
}

bool isStepperMoving() {
    if (!stepper) return false;
    return stepper->isRunning();
}

void stepperEmergencyStop() {
    if (stepper) {
        stepper->forceStop();
    }
    stepperDisable();
}

// ---------------------------------------------------------------------------
// Homing — 3-phase sequence
// ---------------------------------------------------------------------------

void stepperHome() {
    if (!stepper) return;

    g_limit_hit = false;
    stepperEnable();

    // Phase 1: Move toward MIN limit at fast speed
    stepper->setSpeedInHz(static_cast<uint32_t>(HOMING_SPEED_FAST_MM_S * STEPS_PER_MM));
    stepper->move(mmToSteps(-2000.0f));  // large negative move toward min

    while (!g_limit_hit && stepper->isRunning()) {
        vTaskDelay(pdMS_TO_TICKS(1));
    }
    stepper->forceStopAndNewPosition(0);

    // Phase 2: Back off from the switch
    g_limit_hit = false;
    stepper->setSpeedInHz(static_cast<uint32_t>(HOMING_SPEED_SLOW_MM_S * STEPS_PER_MM));
    stepper->move(mmToSteps(HOMING_BACKOFF_MM));

    while (stepper->isRunning()) {
        vTaskDelay(pdMS_TO_TICKS(1));
    }

    // Phase 3: Re-approach slowly for precision
    g_limit_hit = false;
    stepper->move(mmToSteps(-HOMING_BACKOFF_MM * 2.0f));

    while (!g_limit_hit && stepper->isRunning()) {
        vTaskDelay(pdMS_TO_TICKS(1));
    }

    // Set this position as zero
    stepper->forceStopAndNewPosition(0);
    g_limit_hit = false;

    // Restore normal speed
    stepper->setSpeedInHz(static_cast<uint32_t>(100.0f * STEPS_PER_MM));  // 100 mm/s default
}

// ===========================================================================
// NATIVE_TEST stubs
// ===========================================================================
#else

static float  stub_position_mm = 0.0f;
static float  stub_target_mm   = 0.0f;
static bool   stub_moving      = false;
static bool   stub_enabled     = false;

int32_t mmToSteps(float mm) {
    return static_cast<int32_t>(mm * STEPS_PER_MM);
}

float stepsToMm(int32_t steps) {
    return static_cast<float>(steps) * MM_PER_STEP;
}

void stepperInit() {
    stub_position_mm = 0.0f;
    stub_target_mm   = 0.0f;
    stub_moving      = false;
    stub_enabled     = false;
    g_limit_hit      = false;
}

void stepperEnable()  { stub_enabled = true;  }
void stepperDisable() { stub_enabled = false; }

void setStepperTarget(float position_mm) {
    stub_target_mm   = position_mm;
    stub_position_mm = position_mm;  // instant move in test
    stub_moving      = false;
}

float getStepperPositionMm() { return stub_position_mm; }
bool  isStepperMoving()      { return stub_moving; }

void stepperEmergencyStop() {
    stub_moving  = false;
    stub_enabled = false;
}

void stepperHome() {
    stub_position_mm = 0.0f;
    stub_target_mm   = 0.0f;
    g_limit_hit      = false;
}

#endif // NATIVE_TEST
