#pragma once
#include "dji_types.h"
#include "can_bus_interface.h"

namespace dji {

class DjiGimbal {
public:
    DjiGimbal();
    ~DjiGimbal();

    // --- Lifecycle ---

    /// Initialize the library and begin the connection handshake.
    /// @param can  Pointer to a CanBusInterface implementation (caller owns lifetime)
    /// @return true if initialization succeeded
    bool begin(CanBusInterface* can);

    /// Process incoming CAN frames, manage state machine, handle timers.
    /// Call from the main loop or a FreeRTOS task at >= 100 Hz.
    /// @param now_ms Current time in milliseconds (e.g. millis() on Arduino).
    ///               Passed explicitly so the library has no platform dependencies.
    void update(uint32_t now_ms);

    /// Shut down: transition to DISCONNECTED, release internal state.
    void end();

    // --- State ---

    GimbalState getState() const;
    Attitude getAttitude() const;
    Attitude getJointAngles() const;
    GimbalTelemetry getTelemetry() const;
    uint32_t getTimeSinceLastTelemetry(uint32_t now_ms) const;

    // --- Commands (Phase 1) ---

    /// Move to an absolute position.
    /// @param duration_ms Motion duration (min 100, max 25500, resolution 100ms)
    CommandResult setPosition(float pan_deg, float tilt_deg, float roll_deg,
                              uint16_t duration_ms);

    /// Move only yaw and pitch (roll unchanged).
    CommandResult setPanTilt(float pan_deg, float tilt_deg, uint16_t duration_ms);

    /// Trigger a camera action.
    CommandResult cameraControl(CameraAction action);

    // --- Commands (Phase 2) ---

    /// Set rotation speed (degrees/second, 0-360). Must be called < 500ms apart.
    CommandResult setSpeed(float pan_speed, float tilt_speed, float roll_speed);

    /// Stop all speed-controlled rotation.
    CommandResult stopSpeed();

    /// Query gimbal info. Result available via getTelemetry() after next update().
    CommandResult requestInfo();

    // --- Commands (Phase 3) ---

    CommandResult setAngleLimits(uint8_t pitch_max, uint8_t pitch_min,
                                uint8_t yaw_max, uint8_t yaw_min,
                                uint8_t roll_max, uint8_t roll_min);

    CommandResult setMotorStiffness(uint8_t pitch, uint8_t yaw, uint8_t roll);

    // --- Callbacks ---

    void onStateChange(StateChangeCallback cb);
    void onAttitudeUpdate(AttitudeCallback cb);
    void onTelemetryUpdate(TelemetryCallback cb);

private:
    struct Impl;
    Impl* _impl;
};

}  // namespace dji
