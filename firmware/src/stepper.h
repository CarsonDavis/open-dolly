#pragma once

#include <cstdint>

// ---------------------------------------------------------------------------
// Stepper constants
// ---------------------------------------------------------------------------

static constexpr float STEPS_PER_MM = 80.0f;    // 3200 microsteps / 40mm per rev
static constexpr float MM_PER_STEP  = 0.0125f;  // 1 / 80

// ---------------------------------------------------------------------------
// API
// ---------------------------------------------------------------------------

void  stepperInit();
void  stepperEnable();
void  stepperDisable();
void  setStepperTarget(float position_mm);
float getStepperPositionMm();

int32_t mmToSteps(float mm);
float   stepsToMm(int32_t steps);

void  stepperHome();
bool  isStepperMoving();
void  stepperEmergencyStop();

// ---------------------------------------------------------------------------
// Limit switch
// ---------------------------------------------------------------------------

extern volatile bool g_limit_hit;

#ifndef NATIVE_TEST
void IRAM_ATTR onLimitSwitch();
#endif
