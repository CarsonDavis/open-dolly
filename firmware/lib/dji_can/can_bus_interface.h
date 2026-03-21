#pragma once
#include <cstdint>
#include <cstddef>

namespace dji {

class CanBusInterface {
public:
    virtual ~CanBusInterface() = default;

    /// Send a standard CAN 2.0 frame.
    virtual bool send(uint32_t id, const uint8_t* data, uint8_t length) = 0;

    /// Receive a standard CAN 2.0 frame (non-blocking).
    virtual bool receive(uint32_t& id, uint8_t* data, uint8_t& length,
                         uint32_t timeout_ms = 0) = 0;
};

}  // namespace dji
