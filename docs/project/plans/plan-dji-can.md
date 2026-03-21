# Implementation Plan: DJI Gimbal CAN Protocol Library

*Status: draft*
*Last updated: 2026-03-20*

## Overview

A portable C++ library implementing the DJI R SDK protocol for controlling RS-series gimbals over CAN bus. The library targets the ESP32-S3 (TWAI peripheral) but has no direct dependency on ESP32 APIs -- all hardware interaction goes through an abstract `CanBusInterface`. This makes the protocol logic testable on a host machine with captured CAN frames.

No one has implemented this protocol on ESP32 before. This is the project's biggest technical risk (see ADR-001, Risk #1).

## Library Architecture

### File Structure

```
lib/dji_gimbal/
    dji_types.h          -- Shared enums, structs, constants
    dji_crc.h            -- CRC16/CRC32 function declarations
    dji_crc.cpp          -- CRC16/CRC32 table-driven implementations
    dji_protocol.h       -- Frame building/parsing declarations
    dji_protocol.cpp     -- Frame assembly, CRC insertion, frame validation
    dji_gimbal.h         -- High-level API: DjiGimbal class
    dji_gimbal.cpp       -- State machine, command dispatch, telemetry parsing
    can_bus_interface.h   -- Abstract hardware interface (pure virtual)
```

### Dependency Graph

```
can_bus_interface.h    <-- no dependencies (pure interface)
        ^
        |
dji_types.h            <-- no dependencies (data types only)
    ^       ^
    |       |
dji_crc.h          dji_protocol.h
dji_crc.cpp            |
    ^                  |
    |                  v
    +--------> dji_protocol.cpp
                       ^
                       |
               dji_gimbal.h
               dji_gimbal.cpp
```

### Design Principles

1. **No ESP32 headers anywhere in `lib/dji_gimbal/`.** The library compiles on any C++11 toolchain.
2. **No dynamic allocation.** All buffers are fixed-size. The maximum DJI R SDK frame is 256 bytes (10-bit length field, max value 1023, but practical max is ~30 bytes for any known command).
3. **No threads.** The library is single-threaded and poll-based. The caller invokes `update()` from their main loop or a FreeRTOS task. This avoids portability issues with threading primitives.
4. **No blocking calls.** `send` and `receive` on the CAN interface have timeout semantics; the library never spins.

---

## Frame Format

### Byte Layout

Every DJI R SDK packet has this structure:

| Offset | Size | Field | Description |
|--------|------|-------|-------------|
| 0 | 1 | SOF | Start of frame, always `0xAA` |
| 1 | 2 | Ver/Length | Bits [15:10] = version (0), bits [9:0] = total frame length. **Little-endian.** |
| 3 | 1 | CmdType | Bits [7:6] = reserved (0), bit [5] = frame type (0=command, 1=reply), bits [4:0] = reply type |
| 4 | 1 | ENC | Bits [7:5] = encryption type (0=none), bits [4:0] = supplementary bytes length |
| 5 | 3 | RES | Reserved, all zeros |
| 8 | 2 | SEQ | Sequence number, **little-endian**. Wraps from 0x0002 to 0xFFFD. |
| 10 | 2 | CRC-16 | CRC16 over bytes [0..9] (the header). **Little-endian.** |
| 12 | 1 | CmdSet | Command set (0x0E = gimbal, 0x0D = camera) |
| 13 | 1 | CmdID | Command ID within the set |
| 14 | n | CmdData | Payload bytes (command-specific) |
| 12+n | 4 | CRC-32 | CRC32 over bytes [0..11+n] (everything except the CRC32 itself). **Little-endian.** |

**Total frame length** = 12 (header) + 2 (CmdSet + CmdID) + n (payload) + 4 (CRC32) = 18 + n.

The Ver/Length field stores the total frame length in its lower 10 bits. The upper 6 bits are the version number (always 0). Since the field is little-endian:
- Byte 1 = length & 0xFF
- Byte 2 = ((version & 0x3F) << 2) | ((length >> 8) & 0x03)

### CmdType Field Detail

For command frames sent from the controller to the gimbal:
- Bit 5 = 0 (command frame)
- Bits [4:0] = reply request: 0x00 = no reply needed, 0x01 = reply optional, 0x03 = reply required

The demo software uses `0x03` for all commands (reply required after data is sent).

For reply frames from the gimbal:
- Bit 5 = 1 (reply frame), so CmdType = 0x20 for a standard reply

### Sequence Number Management

- Initial value: `0x0002`
- Increment by 1 for each frame sent
- When reaching `0xFFFD`, wrap back to `0x0002`
- The gimbal echoes the sequence number in reply frames, allowing request/response correlation

### CAN Frame Segmentation

DJI R SDK packets are larger than 8 bytes (the CAN 2.0 data field limit). The protocol splits packets into consecutive CAN frames:

- Each CAN frame carries up to 8 bytes of the SDK packet
- The last CAN frame may have fewer than 8 bytes (its DLC reflects the actual remaining byte count)
- All CAN frames use the same CAN ID (0x223 for TX, 0x222 for RX)
- Frames must be sent in order with no interleaving
- The receiver reassembles by concatenating CAN frame payloads until `frame_length` bytes are collected

Example: A 26-byte SDK packet requires ceil(26/8) = 4 CAN frames (8+8+8+2 bytes).

### Example Frame (from SDK documentation)

Position command to move to yaw=3.2deg, roll=4.8deg, pitch=6.4deg in 2.0s:

```
AA 1A 00 03 00 00 00 00 22 11 A2 42 0E 00 20 00 30 00 40 00 01 14 7B 40 97 BE
|  |     |  |  |     |  |     |     |  |  |     |     |     |  |  |           |
|  |     |  |  |     |  |     |     |  |  |     |     |     |  |  +-- CRC32   |
|  |     |  |  |     |  |     |     |  |  |     |     |     |  +-- time=0x14 (20 = 2.0s)
|  |     |  |  |     |  |     |     |  |  |     |     |     +-- ctrl=0x01 (absolute, yaw+roll+pitch valid)
|  |     |  |  |     |  |     |     |  |  |     |     +-- pitch=0x0040 (64 = 6.4 deg)
|  |     |  |  |     |  |     |     |  |  |     +-- roll=0x0030 (48 = 4.8 deg)
|  |     |  |  |     |  |     |     |  |  +-- yaw=0x0020 (32 = 3.2 deg)
|  |     |  |  |     |  |     |     |  +-- CmdID=0x00 (position control)
|  |     |  |  |     |  |     |     +-- CmdSet=0x0E (gimbal)
|  |     |  |  |     |  |     +-- CRC16=0x42A2
|  |     |  |  |     |  +-- SEQ=0x1122
|  |     |  |  |     +-- RES (3 bytes, all 0)
|  |     |  |  +-- ENC=0x00 (no encryption)
|  |     |  +-- CmdType=0x03 (command, reply required)
|  |     +-- length high bits (0x00, version=0, length bits [9:8]=0)
|  +-- length low byte (0x1A = 26)
+-- SOF=0xAA
```

This 26-byte packet is split into 4 CAN frames on ID 0x223:
1. `[AA 1A 00 03 00 00 00 00]` (8 bytes)
2. `[22 11 A2 42 0E 00 20 00]` (8 bytes)
3. `[30 00 40 00 01 14 7B 40]` (8 bytes)
4. `[97 BE]` (2 bytes)

---

## CRC Algorithms

Both CRC algorithms use **table-driven** implementations with pre-computed 256-entry lookup tables. The tables are identical between the Python SDK (`SDKCRC.py`) and C reference implementations (`custom_crc16.c`, `custom_crc32.c`).

### CRC16

| Parameter | Value |
|-----------|-------|
| Width | 16 |
| Polynomial | 0x8005 |
| Init (reflected) | 0x3AA3 |
| Reflect In | Yes |
| Reflect Out | Yes |
| XOR Out | 0x0000 |

**Scope:** Bytes [0..9] of the frame header (10 bytes). The 2-byte result is stored little-endian at offset 10.

```cpp
uint16_t dji_crc16(const uint8_t* data, size_t length) {
    uint16_t crc = 0x3AA3;
    for (size_t i = 0; i < length; i++) {
        crc = (crc >> 8) ^ CRC16_TABLE[(crc ^ data[i]) & 0xFF];
    }
    return crc;
}
```

The `CRC16_TABLE` is the 256-entry table from `SDKCRC.py` / `custom_crc16.c` (reproduced in full in the implementation).

### CRC32

| Parameter | Value |
|-----------|-------|
| Width | 32 |
| Polynomial | 0x04C11DB7 |
| Init (reflected) | 0x00003AA3 |
| Reflect In | Yes |
| Reflect Out | Yes |
| XOR Out | 0x00000000 |

**Scope:** All bytes of the frame except the last 4 (the CRC32 field itself). The 4-byte result is stored little-endian at the end of the frame.

```cpp
uint32_t dji_crc32(const uint8_t* data, size_t length) {
    uint32_t crc = 0x00003AA3;
    for (size_t i = 0; i < length; i++) {
        crc = (crc >> 8) ^ CRC32_TABLE[(crc ^ data[i]) & 0xFF];
    }
    return crc;
}
```

The `CRC32_TABLE` is the 256-entry table from `SDKCRC.py` / `custom_crc32.c`.

### Test Vectors

From the SDK documentation command sample (position control frame):

**CRC16 test vector:**
- Input: `AA 1A 00 03 00 00 00 00 22 11` (10 bytes, header before CRC16)
- Expected CRC16: `0x42A2` (stored as bytes `A2 42`)

**CRC32 test vector:**
- Input: `AA 1A 00 03 00 00 00 00 22 11 A2 42 0E 00 20 00 30 00 40 00 01 14` (22 bytes, full frame minus CRC32)
- Expected CRC32: `0xBE9740xx` -- stored as bytes `7B 40 97 BE` (little-endian)

Additional test vectors should be generated by running the Python SDK's `calc_crc16` and `calc_crc32` functions against known inputs and recording the results. At minimum:
1. Empty-payload commands (e.g., enable push: just CmdSet + CmdID + one data byte)
2. The example position command above
3. A speed control command with known values

### Note on Init Value Equivalence

The C reference headers (generated by pycrc) show `XorIn = 0xC55C` for CRC16 and `XorIn = 0xC55C0000` for CRC32. But `crc_init()` returns `0x3AA3` for both. This is because pycrc reflects XorIn when ReflectIn=True: `reflect(0xC55C, 16) = 0x3AA3` and `reflect(0xC55C0000, 32) = 0x00003AA3`. The Python SDK uses `0x3AA3` directly. Our implementation uses the reflected init values (`0x3AA3`) since we work with pre-reflected lookup tables.

---

## CAN Bus Configuration

| Parameter | Value |
|-----------|-------|
| Baud rate | 1 Mbps |
| Frame type | Standard CAN 2.0 (11-bit identifier) |
| TX CAN ID | 0x223 (controller to gimbal) |
| RX CAN ID | 0x222 (gimbal to controller) |
| Max data per CAN frame | 8 bytes |

**ESP32-S3 TWAI configuration:**
- Use the ESP-IDF TWAI driver (available through Arduino framework)
- Set acceptance filter to receive only CAN ID 0x222
- TX and RX pins are user-configurable GPIO (no conflict on S3, unlike original ESP32)
- External CAN transceiver required: TJA1051 or SN65HVD230

**Physical connection:**
- ESP32-S3 GPIO (TX) --> TJA1051 TXD
- ESP32-S3 GPIO (RX) <-- TJA1051 RXD
- TJA1051 CANH/CANL <--> DJI R Focus Wheel NATO port (pins 4/2)
- DJI NATO port also provides 8V power (pin 1) and GND (pin 6)

---

## Command Set

### Phase 1: Minimum Viable (handshake + position control)

These three commands are sufficient to prove the protocol works and move the gimbal to a target position. Implement and validate these before anything else.

#### 1. Enable Telemetry Push

**CmdSet = 0x0E, CmdID = 0x07**

Enables the gimbal's continuous telemetry stream. This is the first command sent after CAN initialization -- the gimbal will not send any data until this command is received.

```cpp
// Command payload
struct PushSettingsCmd {
    uint8_t ctrl_byte;  // 0x00=no operation, 0x01=enable, 0x02=disable
} __attribute__((packed));
```

| Offset | Size | Name | Type | Value |
|--------|------|------|------|-------|
| 0 | 1 | ctrl_byte | uint8_t | 0x01 to enable |

**Frame construction:**
- CmdType = 0x03 (command, reply required)
- Payload = `01`
- Total frame length = 18 + 1 = 19 bytes

**Expected reply:**
- CmdType = 0x20 (reply frame)
- Payload = 1 byte: return code (0x00 = success)

**After success:** The gimbal begins pushing telemetry frames (CmdSet=0x0E, CmdID=0x08) continuously. The push frequency is approximately 50 Hz based on the demo software's receive loop timing.

#### 2. Position Control

**CmdSet = 0x0E, CmdID = 0x00**

Moves the gimbal to an absolute angle.

```cpp
struct PositionControlCmd {
    int16_t yaw_angle;       // Unit: 0.1 degree. Range: -1800 to +1800
    int16_t roll_angle;      // Unit: 0.1 degree. Range: -1800 to +1800 (v2.2), -300 to +300 (v2.5)
    int16_t pitch_angle;     // Unit: 0.1 degree. Range: -1800 to +1800 (v2.2), -560 to +1460 (v2.5)
    uint8_t ctrl_byte;       // See bit field below
    uint8_t time_for_action; // Unit: 0.1 second. Execution time (e.g., 20 = 2.0s)
} __attribute__((packed));
```

**ctrl_byte bit field:**

| Bit | Name | Values |
|-----|------|--------|
| 0 | Control mode | 0 = incremental, 1 = absolute |
| 1 | Yaw axis valid | 0 = valid (apply), 1 = invalid (ignore) |
| 2 | Roll axis valid | 0 = valid (apply), 1 = invalid (ignore) |
| 3 | Pitch axis valid | 0 = valid (apply), 1 = invalid (ignore) |
| 7:4 | Reserved | Must be 0 |

Note the inverted logic: **0 = valid/apply**, 1 = invalid/ignore. To move all three axes in absolute mode, set ctrl_byte = `0x01` (bit 0 = absolute, bits 1-3 = 0 = all valid).

| Offset | Size | Name | Type | Description |
|--------|------|------|------|-------------|
| 0 | 2 | yaw_angle | int16_t LE | Target yaw in 0.1 deg |
| 2 | 2 | roll_angle | int16_t LE | Target roll in 0.1 deg |
| 4 | 2 | pitch_angle | int16_t LE | Target pitch in 0.1 deg |
| 6 | 1 | ctrl_byte | uint8_t | Control flags |
| 7 | 1 | time_for_action | uint8_t | Motion duration in 0.1s |

**Frame construction:**
- CmdType = 0x03
- Payload = 8 bytes
- Total frame length = 18 + 8 = 26 bytes (4 CAN frames)

**Expected reply:**
- Payload = 1 byte: return code (0x00 = success)

**Timing constraint:** Minimum 100ms between position commands (per SDK documentation: `time_for_action` unit is 0.1s and the minimum useful value is 1, meaning 0.1s minimum execution time).

**DJI rate limiting and firmware trajectory walker:** The firmware trajectory walker runs at 100Hz (10ms ticks), but DJI position commands can only be sent at 10Hz (100ms minimum interval). The firmware implements a rate divider: it sends a position command every 10th walker tick. Each position command uses `time_for_action = 1` (0.1 seconds = 100ms) so the gimbal interpolates smoothly between updates. This constant (`DJI_CMD_INTERVAL_TICKS = 10`) should be validated during hardware testing and may need tuning — if the gimbal's internal interpolation produces visible stepping, the interval or `time_for_action` can be adjusted.

#### 3. Parse Telemetry Push

**CmdSet = 0x0E, CmdID = 0x08**

The gimbal pushes this frame continuously after telemetry is enabled. This is **not** a command we send -- it is a frame we receive and parse.

```cpp
struct TelemetryPushData {
    uint8_t  ctrl_byte;          // Bit 0: angle info valid, bit 1: limit info valid, bit 2: motor stiffness valid
    int16_t  yaw_attitude;       // Attitude angle, unit: 0.1 deg
    int16_t  roll_attitude;      // Attitude angle, unit: 0.1 deg
    int16_t  pitch_attitude;     // Attitude angle, unit: 0.1 deg
    int16_t  yaw_joint;          // Joint angle, unit: 0.1 deg
    int16_t  roll_joint;         // Joint angle, unit: 0.1 deg
    int16_t  pitch_joint;        // Joint angle, unit: 0.1 deg
    uint8_t  pitch_max;          // Max tilt axis angle (0-179, v2.2) or (0-145, v2.5)
    uint8_t  pitch_min;          // Min tilt axis angle (0-179, v2.2) or (0-55, v2.5)
    uint8_t  yaw_max;            // Max pan axis angle (0-179)
    uint8_t  yaw_min;            // Min pan axis angle (0-179)
    uint8_t  roll_max;           // Max roll axis angle (0-179, v2.2) or (0-30, v2.5)
    uint8_t  roll_min;           // Min roll axis angle (0-179, v2.2) or (0-30, v2.5)
    uint8_t  pitch_stiffness;    // 0-100
    uint8_t  yaw_stiffness;      // 0-100
    uint8_t  roll_stiffness;     // 0-100
} __attribute__((packed));
```

| Offset | Size | Name | Type | Description |
|--------|------|------|------|-------------|
| 0 | 1 | ctrl_byte | uint8_t | Bit 0: angle valid, bit 1: limits valid, bit 2: stiffness valid |
| 1 | 2 | yaw_attitude | int16_t LE | Attitude angle in 0.1 deg |
| 3 | 2 | roll_attitude | int16_t LE | Attitude angle in 0.1 deg |
| 5 | 2 | pitch_attitude | int16_t LE | Attitude angle in 0.1 deg |
| 7 | 2 | yaw_joint | int16_t LE | Joint angle in 0.1 deg |
| 9 | 2 | roll_joint | int16_t LE | Joint angle in 0.1 deg |
| 11 | 2 | pitch_joint | int16_t LE | Joint angle in 0.1 deg |
| 13 | 1 | pitch_max | uint8_t | Degrees |
| 14 | 1 | pitch_min | uint8_t | Degrees |
| 15 | 1 | yaw_max | uint8_t | Degrees |
| 16 | 1 | yaw_min | uint8_t | Degrees |
| 17 | 1 | roll_max | uint8_t | Degrees |
| 18 | 1 | roll_min | uint8_t | Degrees |
| 19 | 1 | pitch_stiffness | uint8_t | 0-100 |
| 20 | 1 | yaw_stiffness | uint8_t | 0-100 |
| 21 | 1 | roll_stiffness | uint8_t | 0-100 |

Total data segment = 22 bytes. Total frame = 18 + 22 = 40 bytes = 5 CAN frames.

**Parsing the ctrl_byte:** Check bit 0 before reading attitude/joint angles. If bit 0 is 0, the angle values are invalid and should be ignored. The demo software checks this explicitly.

#### 4. Camera Control

**CmdSet = 0x0D, CmdID = 0x00**

Trigger camera functions (shutter, video start/stop, focus). Camera shutter/record is a V1 feature since the DJI CAN protocol makes it straightforward — the command is a single 2-byte payload with no complex state management.

```cpp
struct CameraControlCmd {
    uint16_t command;  // Camera control command, little-endian
} __attribute__((packed));
```

| Command Value | Description |
|---------------|-------------|
| 0x0001 | Shutter (take photo) |
| 0x0002 | Stop shuttering |
| 0x0003 | Start recording |
| 0x0004 | Stop recording |
| 0x0005 | Center focus |
| 0x000B | End center focus |

**Expected reply:** 1 byte return code.

### Phase 2: Speed Control and Information

#### 5. Speed Control

**CmdSet = 0x0E, CmdID = 0x01**

Continuous rotation at a specified speed. The command expires after 0.5s and must be resent periodically to maintain rotation.

```cpp
struct SpeedControlCmd {
    int16_t yaw_speed;    // Unit: 0.1 deg/s. Range: 0 to 3600 (0-360 deg/s)
    int16_t roll_speed;   // Unit: 0.1 deg/s. Range: 0 to 3600
    int16_t pitch_speed;  // Unit: 0.1 deg/s. Range: 0 to 3600
    uint8_t ctrl_byte;    // Bit 7: 0=release, 1=take over. Bit 3: camera focal length. Bits 6:4, 2:0 reserved (0).
} __attribute__((packed));
```

| Offset | Size | Name | Type | Description |
|--------|------|------|------|-------------|
| 0 | 2 | yaw_speed | int16_t LE | Speed in 0.1 deg/s |
| 2 | 2 | roll_speed | int16_t LE | Speed in 0.1 deg/s |
| 4 | 2 | pitch_speed | int16_t LE | Speed in 0.1 deg/s |
| 6 | 1 | ctrl_byte | uint8_t | Control flags |

**ctrl_byte for speed control:**

| Bit | Name | Values |
|-----|------|--------|
| 7 | Control bit | 0 = release speed control, 1 = take over speed control |
| 6:4 | Reserved | Must be 0 |
| 3 | Camera focal length | 0 = consider focal length impact on speed, 1 = ignore |
| 2:0 | Reserved | Must be 0 |

The demo software uses ctrl_byte = `0x80` (take over speed control, consider focal length).

**To stop rotation:** Send speed command with all speeds = 0, or send with ctrl_byte bit 7 = 0 (release).

**Timing:** Must resend within 500ms or the gimbal releases speed control automatically (safety feature).

**Expected reply:**
- Payload = 1 byte: return code

#### 6. Get Gimbal Information

**CmdSet = 0x0E, CmdID = 0x02**

Query current attitude or joint angles.

```cpp
// Command payload
struct GetInfoCmd {
    uint8_t ctrl_byte;  // 0x00=no op, 0x01=get attitude angles, 0x02=get joint angles
} __attribute__((packed));

// Reply payload
struct GetInfoReply {
    uint8_t return_code;  // 0x00 success
    uint8_t data_type;    // 0x00=not ready, 0x01=attitude, 0x02=joint
    int16_t yaw;          // Unit: 0.1 deg
    int16_t roll;         // Unit: 0.1 deg
    int16_t pitch;        // Unit: 0.1 deg
} __attribute__((packed));
```

| Offset | Size | Name | Type | Description |
|--------|------|------|------|-------------|
| **Command** | | | | |
| 0 | 1 | ctrl_byte | uint8_t | 0x01=attitude, 0x02=joint |
| **Reply** | | | | |
| 0 | 1 | return_code | uint8_t | 0x00=success |
| 1 | 1 | data_type | uint8_t | 0x00=not ready, 0x01=attitude, 0x02=joint |
| 2 | 2 | yaw | int16_t LE | Angle in 0.1 deg |
| 4 | 2 | roll | int16_t LE | Angle in 0.1 deg |
| 6 | 2 | pitch | int16_t LE | Angle in 0.1 deg |

### Phase 3: Configuration and Advanced

#### 7. Get/Set Angle Limits

**Set: CmdSet = 0x0E, CmdID = 0x03**

```cpp
struct SetAngleLimitCmd {
    uint8_t ctrl_byte;   // 0x00=no op, 0x01=set limits
    uint8_t pitch_max;   // Degrees (0-145 for v2.5, 0-179 for v2.2)
    uint8_t pitch_min;   // Degrees (0-55 for v2.5, 0-179 for v2.2)
    uint8_t yaw_max;     // Degrees (0-179)
    uint8_t yaw_min;     // Degrees (0-179)
    uint8_t roll_max;    // Degrees (0-30 for v2.5, 0-179 for v2.2)
    uint8_t roll_min;    // Degrees (0-30 for v2.5, 0-179 for v2.2)
} __attribute__((packed));
```

**Get: CmdSet = 0x0E, CmdID = 0x04**

Send ctrl_byte = 0x01 to request limits. Reply contains return_code followed by 6 limit bytes (same layout as set command minus ctrl_byte).

#### 8. Get/Set Motor Stiffness

**Set: CmdSet = 0x0E, CmdID = 0x05**

```cpp
struct SetMotorStiffnessCmd {
    uint8_t ctrl_byte;        // 0x01 to set
    uint8_t pitch_stiffness;  // 0-100
    uint8_t roll_stiffness;   // 0-100
    uint8_t yaw_stiffness;    // 0-100
} __attribute__((packed));
```

**Get: CmdSet = 0x0E, CmdID = 0x06**

Send ctrl_byte = 0x01. Reply: return_code + 3 stiffness bytes.

#### 9. Focus Motor Control (v2.5 only, Phase 3)

**CmdSet = 0x0E, CmdID = 0x12**

Position control of the DJI focus motor. Push frequency is 100 Hz.

```cpp
struct FocusMotorCmd {
    uint8_t  command_sub_id;   // 0x01 = focus motor position control
    uint8_t  control_type;     // 0x00 = focus control
    uint8_t  data_length;      // 0x02 = two-byte position
    uint16_t absolute_position; // 0-4095
} __attribute__((packed));
```

This command has no reply frame -- it is fire-and-forget at 100 Hz.

Focus motor calibration uses `command_sub_id = 0x02` with additional calibration fields. Position query uses `command_sub_id = 0x15`.

---

## State Machine

### States

```
                    begin()
                      |
                      v
    +----------> DISCONNECTED
    |                 |
    | end()           | init CAN + send enable push
    |                 v
    +----------- CONNECTING --------+
    |                 |             |
    |                 | first       | timeout (no response
    |                 | telemetry   | after N retries)
    |                 | received    |
    |                 v             v
    +------------ ACTIVE ------> ERROR
                      ^             |
                      |             | retry timer elapsed
                      +-------------+
```

### State Definitions

| State | Description | Entry Action | Exit Action |
|-------|-------------|-------------|-------------|
| DISCONNECTED | Library not started or explicitly shut down | None | None |
| CONNECTING | CAN initialized, enable-push command sent, waiting for first telemetry | Send enable-push command, start connect timeout | Cancel timeout |
| ACTIVE | Receiving telemetry, commands accepted | Invoke onStateChange callback | None |
| ERROR | Communication lost (telemetry timeout) or unrecoverable error | Invoke onStateChange callback, start retry timer | Cancel retry timer |

### Transitions

| From | To | Trigger | Action |
|------|----|---------|--------|
| DISCONNECTED | CONNECTING | `begin()` called | Configure CAN, send enable-push |
| CONNECTING | ACTIVE | First valid telemetry push (CmdSet=0x0E, CmdID=0x08) received | Store attitude, invoke callback |
| CONNECTING | ERROR | No telemetry within `CONNECT_TIMEOUT_MS` | Log error |
| ACTIVE | ERROR | No telemetry within `TELEMETRY_TIMEOUT_MS` | Log error |
| ERROR | CONNECTING | `RETRY_INTERVAL_MS` elapsed, retries < `MAX_RETRIES` | Re-send enable-push |
| ERROR | DISCONNECTED | Retries exhausted (`MAX_RETRIES` reached) | Invoke callback with final error |
| Any | DISCONNECTED | `end()` called | Release CAN resources |

### Timing Constants

| Constant | Value | Rationale |
|----------|-------|-----------|
| `CONNECT_TIMEOUT_MS` | 3000 | Allow 3 seconds for gimbal to respond after power-on |
| `TELEMETRY_TIMEOUT_MS` | 500 | Telemetry push is ~50 Hz; 500ms = 25 missed frames |
| `RETRY_INTERVAL_MS` | 2000 | Wait 2 seconds between reconnection attempts |
| `MAX_RETRIES` | 5 | Give up after 5 failed reconnection attempts |
| `MIN_POSITION_CMD_INTERVAL_MS` | 100 | Minimum time between position commands (SDK spec) |
| `SPEED_CMD_REFRESH_MS` | 400 | Resend speed commands before 500ms timeout |

### Processing Model

The `update()` method is called from the main loop (or a FreeRTOS task). Each call:

1. Reads all available CAN frames from the interface (non-blocking)
2. Appends bytes to the reassembly buffer
3. When a complete SDK frame is assembled:
   a. Validates CRC16 (header) and CRC32 (full frame)
   b. Identifies frame type (reply vs push)
   c. For replies: matches sequence number to pending command, invokes completion
   d. For pushes: parses telemetry data, updates state, invokes callback
4. Checks timers (telemetry timeout, retry timer, speed command refresh)
5. Sends any pending commands

---

## Public API

### Types (`dji_types.h`)

```cpp
#pragma once
#include <cstdint>
#include <cstddef>

namespace dji {

// --- Enums ---

enum class GimbalState : uint8_t {
    DISCONNECTED,
    CONNECTING,
    ACTIVE,
    ERROR
};

enum class CommandResult : uint8_t {
    SUCCESS          = 0x00,
    PARSE_ERROR      = 0x01,
    EXECUTION_FAILED = 0x02,
    UNDEFINED_ERROR  = 0xFF,
    TIMEOUT          = 0xFE,  // Library-defined: no reply within timeout
    NOT_CONNECTED    = 0xFD   // Library-defined: gimbal not in ACTIVE state
};

enum class CameraAction : uint16_t {
    SHUTTER        = 0x0001,
    STOP_SHUTTER   = 0x0002,
    START_RECORD   = 0x0003,
    STOP_RECORD    = 0x0004,
    CENTER_FOCUS   = 0x0005,
    END_FOCUS      = 0x000B
};

// --- Data Structs ---

struct Attitude {
    float yaw;    // Degrees (-180.0 to +180.0)
    float roll;   // Degrees
    float pitch;  // Degrees
};

struct GimbalTelemetry {
    bool     angles_valid;
    Attitude attitude;       // Attitude angles (gimbal orientation in world frame)
    Attitude joint;          // Joint angles (motor positions)

    bool     limits_valid;
    uint8_t  pitch_max;      // Degrees
    uint8_t  pitch_min;
    uint8_t  yaw_max;
    uint8_t  yaw_min;
    uint8_t  roll_max;
    uint8_t  roll_min;

    bool     stiffness_valid;
    uint8_t  pitch_stiffness; // 0-100
    uint8_t  yaw_stiffness;
    uint8_t  roll_stiffness;
};

struct GimbalInfo {
    uint32_t device_id;
    uint32_t version;       // 0xAABBCCDD = version AA.BB.CC.DD
};

// --- Callback Types ---

using StateChangeCallback   = void (*)(GimbalState old_state, GimbalState new_state);
using AttitudeCallback       = void (*)(const Attitude& attitude);
using TelemetryCallback      = void (*)(const GimbalTelemetry& telemetry);

// --- Constants ---

static constexpr uint32_t CAN_TX_ID = 0x223;
static constexpr uint32_t CAN_RX_ID = 0x222;
static constexpr uint32_t CAN_BAUD_RATE = 1000000;

static constexpr size_t MAX_FRAME_SIZE = 256;
static constexpr size_t CAN_FRAME_MAX_DATA = 8;

}  // namespace dji
```

### Hardware Abstraction (`can_bus_interface.h`)

```cpp
#pragma once
#include <cstdint>
#include <cstddef>

namespace dji {

/// Abstract CAN bus interface. The library calls these methods to
/// send and receive raw CAN frames. The ESP32 TWAI driver implements
/// this interface; test harnesses implement it with captured data.
class CanBusInterface {
public:
    virtual ~CanBusInterface() = default;

    /// Send a standard CAN 2.0 frame.
    /// @param id      11-bit CAN identifier
    /// @param data    Pointer to data bytes (up to 8)
    /// @param length  Number of data bytes (0-8)
    /// @return true if the frame was accepted by the CAN controller
    virtual bool send(uint32_t id, const uint8_t* data, uint8_t length) = 0;

    /// Receive a standard CAN 2.0 frame (non-blocking).
    /// @param[out] id      CAN identifier of received frame
    /// @param[out] data    Buffer for received data (must be >= 8 bytes)
    /// @param[out] length  Number of data bytes received
    /// @param timeout_ms   Maximum time to wait (0 = poll without blocking)
    /// @return true if a frame was received, false if timeout/empty
    virtual bool receive(uint32_t& id, uint8_t* data, uint8_t& length,
                         uint32_t timeout_ms = 0) = 0;
};

}  // namespace dji
```

### Gimbal Controller (`dji_gimbal.h`)

```cpp
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
    /// @return true if initialization succeeded (CAN interface valid)
    bool begin(CanBusInterface* can);

    /// Process incoming CAN frames, manage state machine, handle timers.
    /// Call this from the main loop or a FreeRTOS task at >= 100 Hz.
    void update();

    /// Shut down: transition to DISCONNECTED, release internal state.
    void end();

    // --- State ---

    /// Current connection state.
    GimbalState getState() const;

    /// Most recent attitude angles (world-frame orientation).
    /// Only valid when state == ACTIVE and telemetry has been received.
    Attitude getAttitude() const;

    /// Most recent joint angles (motor positions).
    Attitude getJointAngles() const;

    /// Full telemetry snapshot (angles + limits + stiffness).
    GimbalTelemetry getTelemetry() const;

    /// Milliseconds since last telemetry was received.
    /// Returns UINT32_MAX if no telemetry has ever been received.
    uint32_t getTimeSinceLastTelemetry() const;

    // --- Commands (Phase 1) ---

    /// Move to an absolute position.
    /// @param pan_deg    Yaw angle in degrees (-180.0 to +180.0)
    /// @param tilt_deg   Pitch angle in degrees
    /// @param roll_deg   Roll angle in degrees
    /// @param duration_ms Motion duration in milliseconds (min 100, max 25500)
    /// @return SUCCESS if command was sent, NOT_CONNECTED if state != ACTIVE
    CommandResult setPosition(float pan_deg, float tilt_deg, float roll_deg,
                              uint16_t duration_ms);

    /// Move only yaw and pitch (roll unchanged). Convenience wrapper.
    CommandResult setPanTilt(float pan_deg, float tilt_deg, uint16_t duration_ms);

    /// Trigger a camera action (photo, video, focus). Phase 1 — straightforward
    /// 2-byte CAN command with no complex state management.
    CommandResult cameraControl(CameraAction action);

    // --- Commands (Phase 2) ---

    /// Set rotation speed for continuous motion.
    /// All speeds in degrees/second (0 to 360). Set all to 0 to stop.
    /// Must be called periodically (< 500ms) to maintain rotation.
    CommandResult setSpeed(float pan_speed, float tilt_speed, float roll_speed);

    /// Stop all speed-controlled rotation immediately.
    CommandResult stopSpeed();

    /// Query gimbal info (device ID, firmware version). Result delivered via callback.
    CommandResult requestInfo();

    // --- Commands (Phase 3) ---

    /// Set angle limits. All values in degrees.
    CommandResult setAngleLimits(uint8_t pitch_max, uint8_t pitch_min,
                                uint8_t yaw_max, uint8_t yaw_min,
                                uint8_t roll_max, uint8_t roll_min);

    /// Set motor stiffness. All values 0-100.
    CommandResult setMotorStiffness(uint8_t pitch, uint8_t yaw, uint8_t roll);

    // --- Callbacks ---

    /// Register a callback for state transitions.
    void onStateChange(StateChangeCallback cb);

    /// Register a callback for attitude updates (called on every telemetry push).
    void onAttitudeUpdate(AttitudeCallback cb);

    /// Register a callback for full telemetry updates.
    void onTelemetryUpdate(TelemetryCallback cb);

private:
    // Internal state - implementation details in dji_gimbal.cpp
    struct Impl;
    Impl* _impl;
};

}  // namespace dji
```

### Protocol Layer (`dji_protocol.h`)

```cpp
#pragma once
#include "dji_types.h"

namespace dji {

/// Build a complete DJI R SDK frame.
/// @param cmd_type   CmdType byte (e.g., 0x03 for command with reply required)
/// @param cmd_set    Command set (e.g., 0x0E for gimbal)
/// @param cmd_id     Command ID
/// @param payload    Command payload data (may be nullptr if payload_len == 0)
/// @param payload_len Length of payload in bytes
/// @param seq        Sequence number
/// @param[out] frame Output buffer (must be >= 18 + payload_len bytes)
/// @return Total frame length, or 0 on error
size_t buildFrame(uint8_t cmd_type, uint8_t cmd_set, uint8_t cmd_id,
                  const uint8_t* payload, size_t payload_len,
                  uint16_t seq, uint8_t* frame);

/// Validate a received frame's CRC16 (header) and CRC32 (full frame).
/// @param frame      Complete frame bytes
/// @param length     Total frame length
/// @return true if both CRCs are valid
bool validateFrame(const uint8_t* frame, size_t length);

/// Extract fields from a validated frame.
/// @param frame      Complete validated frame
/// @param length     Total frame length
/// @param[out] cmd_type  CmdType byte
/// @param[out] seq       Sequence number
/// @param[out] cmd_set   Command set
/// @param[out] cmd_id    Command ID
/// @param[out] payload   Pointer into frame at payload start
/// @param[out] payload_len Payload length (excluding CmdSet, CmdID, CRC32)
/// @return true if extraction succeeded
bool parseFrame(const uint8_t* frame, size_t length,
                uint8_t& cmd_type, uint16_t& seq,
                uint8_t& cmd_set, uint8_t& cmd_id,
                const uint8_t*& payload, size_t& payload_len);

/// Get the next sequence number (manages wrapping 0x0002 -> 0xFFFD).
uint16_t nextSequence();

}  // namespace dji
```

---

## Test Strategy

### Unit Tests (host-side, no hardware)

All unit tests run on the development machine (macOS/Linux) using a standard C++ test framework (Catch2 or GoogleTest). No ESP32 toolchain required.

#### CRC Tests

Verify CRC implementations against known values from the SDK reference.

```cpp
// CRC16: header bytes from the example position command
TEST(CRC16, ExamplePositionHeader) {
    uint8_t header[] = {0xAA, 0x1A, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x22, 0x11};
    EXPECT_EQ(dji_crc16(header, 10), 0x42A2);
}

// CRC32: full frame minus CRC32 bytes
TEST(CRC32, ExamplePositionFrame) {
    uint8_t frame[] = {
        0xAA, 0x1A, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00,
        0x22, 0x11, 0xA2, 0x42, 0x0E, 0x00, 0x20, 0x00,
        0x30, 0x00, 0x40, 0x00, 0x01, 0x14
    };
    EXPECT_EQ(dji_crc32(frame, 22), 0xBE9740xx);  // Verify exact value from SDK
}
```

Additional CRC test vectors to generate:
1. Single-byte payload (enable push command)
2. Zero-length payload edge case
3. Incrementally computed CRC (feed bytes one at a time, verify same result as batch)

#### Frame Building Tests

Construct known commands and verify exact byte sequences.

```cpp
TEST(FrameBuilder, PositionCommand) {
    uint8_t frame[64];
    // Build position command: yaw=3.2, roll=4.8, pitch=6.4, absolute, 2.0s
    uint8_t payload[] = {0x20, 0x00, 0x30, 0x00, 0x40, 0x00, 0x01, 0x14};
    size_t len = buildFrame(0x03, 0x0E, 0x00, payload, 8, 0x1122, frame);
    EXPECT_EQ(len, 26);
    EXPECT_EQ(frame[0], 0xAA);
    EXPECT_EQ(frame[1], 0x1A);  // length = 26
    // ... verify all bytes match expected output
}

TEST(FrameBuilder, EnablePushCommand) {
    uint8_t frame[64];
    uint8_t payload[] = {0x01};
    size_t len = buildFrame(0x03, 0x0E, 0x07, payload, 1, 0x0003, frame);
    EXPECT_EQ(len, 19);  // 18 + 1
}
```

#### Frame Parsing Tests

Feed known response bytes and verify parsed values.

```cpp
TEST(FrameParser, TelemetryPush) {
    // Construct a synthetic telemetry push frame with known attitude values
    // yaw=45.0 deg (450), roll=0.0 (0), pitch=-10.0 deg (-100)
    uint8_t push_data[] = {
        0x01,                    // ctrl_byte: angle info valid
        0xC2, 0x01,              // yaw_attitude = 450 (45.0 deg)
        0x00, 0x00,              // roll_attitude = 0
        0x9C, 0xFF,              // pitch_attitude = -100 (-10.0 deg)
        // ... joint angles, limits, stiffness ...
    };
    // Wrap in a full frame with valid CRC16/CRC32
    // Parse and verify:
    GimbalTelemetry telemetry;
    // ... parse frame ...
    EXPECT_TRUE(telemetry.angles_valid);
    EXPECT_FLOAT_EQ(telemetry.attitude.yaw, 45.0f);
    EXPECT_FLOAT_EQ(telemetry.attitude.pitch, -10.0f);
}
```

#### Frame Reassembly Tests

Simulate receiving multi-frame CAN data.

```cpp
TEST(Reassembly, FourCanFrames) {
    // Feed the 4 CAN frames from the position command example
    // one at a time into the reassembly buffer
    // Verify that after the 4th frame, a complete valid SDK frame is emitted
}

TEST(Reassembly, PartialFrame) {
    // Feed only 2 of 4 frames, then call update()
    // Verify no frame is emitted, no crash, state is waiting
}

TEST(Reassembly, CorruptedFrame) {
    // Feed a frame with invalid CRC16 in the header
    // Verify frame is rejected, parser resets
}
```

#### State Machine Tests

Use a mock CAN bus to simulate the full connection sequence.

```cpp
class MockCanBus : public CanBusInterface {
    std::queue<std::vector<uint8_t>> rx_frames;
    std::vector<std::vector<uint8_t>> tx_log;
public:
    bool send(uint32_t id, const uint8_t* data, uint8_t len) override {
        tx_log.push_back({data, data + len});
        return true;
    }
    bool receive(uint32_t& id, uint8_t* data, uint8_t& len, uint32_t) override {
        if (rx_frames.empty()) return false;
        auto& f = rx_frames.front();
        id = CAN_RX_ID;
        len = f.size();
        memcpy(data, f.data(), len);
        rx_frames.pop();
        return true;
    }
    void enqueueRxFrame(const std::vector<uint8_t>& frame) {
        rx_frames.push(frame);
    }
};

TEST(StateMachine, HandshakeSequence) {
    MockCanBus can;
    DjiGimbal gimbal;
    gimbal.begin(&can);
    EXPECT_EQ(gimbal.getState(), GimbalState::CONNECTING);

    // Verify enable-push command was sent
    EXPECT_FALSE(can.tx_log.empty());

    // Enqueue a simulated enable-push reply (success)
    can.enqueueRxFrame(/* reply frame bytes */);
    gimbal.update();

    // Enqueue a simulated telemetry push
    can.enqueueRxFrame(/* telemetry push frame bytes */);
    gimbal.update();

    EXPECT_EQ(gimbal.getState(), GimbalState::ACTIVE);
}

TEST(StateMachine, TelemetryTimeout) {
    // Connect successfully, then stop sending telemetry
    // After TELEMETRY_TIMEOUT_MS, verify state transitions to ERROR
}

TEST(StateMachine, RetryAndReconnect) {
    // Enter ERROR state, verify retry attempts
    // On successful retry, verify transition back to ACTIVE
}

TEST(StateMachine, MaxRetriesExhausted) {
    // Enter ERROR, exhaust MAX_RETRIES
    // Verify final transition to DISCONNECTED
}
```

#### Sequence Number Tests

```cpp
TEST(Sequence, Wrapping) {
    // Call nextSequence() from 0x0002 up to 0xFFFD
    // Verify it wraps back to 0x0003 (not 0x0002, since 0x0002 is the init)
}

TEST(Sequence, NeverZeroOrOne) {
    // Verify sequence number never returns 0x0000 or 0x0001
    // (reserved values based on SDK init at 0x0002)
}
```

### Integration Test Plan (with hardware)

When the ESP32-S3 + CAN transceiver + DJI gimbal hardware arrives, validate in this exact order. Do not skip steps.

**Step 1: CAN bus electrical verification**
- Connect ESP32-S3 TWAI to CAN transceiver, power on gimbal
- Use TWAI driver in listen-only mode to sniff the bus
- Verify: any frames appear at all? At what rate? What CAN IDs?
- If no frames: check wiring, transceiver, baud rate, termination resistor

**Step 2: Send enable-push, observe response**
- Send the enable-push command (CmdSet=0x0E, CmdID=0x07, Data=0x01)
- Log all received CAN frames (raw hex) for 5 seconds
- Verify: does the gimbal respond? What CAN ID? Does the reply CRC validate?
- If no response: try different CmdType values, check sequence number handling

**Step 3: Parse telemetry**
- After successful enable-push, parse incoming telemetry pushes
- Display attitude angles on serial monitor
- Physically move the gimbal by hand, verify angles change correctly
- Verify: push rate, angle accuracy, ctrl_byte flags

**Step 4: Position control**
- Send a simple position command: yaw=0, pitch=0, roll=0, duration=2s
- Verify: gimbal moves to center position
- Send yaw=45.0 deg, verify gimbal pans
- Test timing: send commands at 100ms intervals, verify no errors

**Step 5: Speed control**
- Send speed command with yaw_speed=30 deg/s
- Verify: gimbal rotates smoothly
- Stop by sending all-zero speeds
- Test 500ms timeout: stop sending, verify gimbal stops on its own

**Step 6: Error recovery**
- Disconnect CAN cable while running, verify ERROR state
- Reconnect, verify automatic recovery
- Power-cycle gimbal, verify reconnection

---

## Risk Register

| # | Risk | Likelihood | Impact | Mitigation |
|---|------|-----------|--------|------------|
| 1 | **No ESP32 + DJI precedent.** No one has run this protocol on any ESP32 variant. Undocumented quirks may exist (timing requirements, initialization sequences, CAN frame ordering). | High | High | Build the CAN handshake test (Step 2 above) as the absolute first firmware milestone. If this fails, we learn fast and cheap. The DJIR_SDK C++ library and DJI-Ronin-RS2-Log-and-Replay Python project serve as working reference implementations to compare against. |
| 2 | **CAN frame segmentation edge cases.** The SDK docs show frames up to ~40 bytes (5 CAN frames). But some responses (telemetry push with all fields valid) or future commands could be longer. The reassembly buffer must handle arbitrary lengths up to the 10-bit max (1023 bytes). | Medium | Medium | Size the reassembly buffer to MAX_FRAME_SIZE (256 bytes). This covers all documented commands with margin. If a frame exceeds this, log the error and skip it rather than crashing. |
| 3 | **Protocol version differences.** RS 2 uses v2.2 (wider angle ranges, no focus motor). RS 4/5 use v2.5 (narrower ranges for roll/pitch, adds CmdSet/CmdID to reply frames, adds focus motor CmdID 0x12). The reply frame format change in v2.5 is especially important -- v2.2 replies have no CmdSet/CmdID, requiring the library to correlate replies by sequence number using the last-sent command (as the demo software does). | Medium | Medium | Implement reply correlation by sequence number (works for both versions). Detect protocol version via the Get Module Version command (CmdID 0x09) during handshake. Store version and use it to gate v2.5-only features (focus motor) and adjust angle range validation. |
| 4 | **Timing sensitivity.** The SDK specifies minimum 100ms between position commands and 500ms speed command timeout. There may be undocumented timing requirements for the initialization sequence (delay between CAN init and first command, minimum gap between enable-push and first position command). | Medium | Low | Add configurable delays at each stage. Start conservative (500ms between init and first command, 200ms between commands), then tighten if testing shows the gimbal is tolerant. |
| 5 | **Gimbal may require specific initialization order.** The demo software sends enable-push as its first and only initialization step. But the gimbal may expect other commands first on different firmware versions, or may need a device ID registration. The DJI documentation mentions a 4-byte Device ID that "must be submitted to DJI for approval." | Low | High | Start with the demo software's exact initialization sequence (just enable-push). If that fails, try sending Get Module Version first (as the DJIR_SDK library does). As a last resort, capture a working USB-CAN adapter session using a CAN bus analyzer and replay the exact initialization sequence. For Device ID, use 0x00000001 (DJI R SDK) since we are acting as an SDK client. |
| 6 | **CAN bus electrical issues.** The ESP32-S3 TWAI peripheral has been validated at 1 Mbps in other projects, but CAN bus reliability depends heavily on wiring (bus termination, cable length, ground reference). The DJI NATO port provides 8V power which needs voltage regulation. | Medium | Low | Use a quality CAN transceiver module (TJA1051 or SN65HVD230) with proper 120-ohm termination. Keep CAN wires short (<30cm for development). Test with an oscilloscope on CANH/CANL if bus errors occur. |
| 7 | **TWAI driver buffer overflow under load.** At 50 Hz telemetry push (~40 bytes each = 5 CAN frames per push = 250 CAN frames/sec), plus replies to our commands, the TWAI RX buffer could overflow if `update()` is not called frequently enough. | Low | Medium | Configure TWAI RX queue to 32+ frames. Call `update()` at >= 100 Hz. Monitor TWAI error counters. If overflow occurs, increase queue size or increase `update()` frequency. |

---

## References

Implementation references within this repository:

| File | What it provides |
|------|-----------------|
| `docs/external/dji-r-sdk/demo-software/protocol/sdk/SDKCRC.py` | Python CRC16/CRC32 with lookup tables and init values. Port these tables directly. |
| `docs/external/dji-r-sdk/demo-software/protocol/sdk/CmdCombine.py` | Frame assembly logic: header construction, CRC16 insertion, CRC32 insertion, sequence number management. |
| `docs/external/dji-r-sdk/demo-software/protocol/sdk/SDKHandle.py` | Frame parsing: reassembly state machine, CRC validation, CmdType dispatch, telemetry push parsing with `struct.unpack`. |
| `docs/external/dji-r-sdk/demo-software/protocol/connection/CANConnection.py` | CAN frame segmentation for send (splitting SDK frames into 8-byte CAN frames). CAN ID configuration (TX=0x223, RX=0x222). |
| `docs/external/dji-r-sdk/demo-software/ui_init/gimbal_window_init.py` | Command construction examples: position control (`struct.pack('<3h2B',...)`), speed control (`struct.pack('<3hB',...)`), camera control, enable-push sequence on connect. |
| `docs/external/dji-r-sdk/docs/custom_crc16.c`, `custom_crc16.h` | C reference CRC16: table-driven, `crc_init()` returns 0x3AA3, same lookup table as Python. Contains test vector (header bytes). |
| `docs/external/dji-r-sdk/docs/custom_crc32.c`, `custom_crc32.h` | C reference CRC32: table-driven, `crc_init()` returns 0x00003AA3, same lookup table as Python. Contains test vector (full position command minus CRC32). |
| `docs/external/dji-r-sdk/docs/DJI_R_SDK_Protocol_v2.2_EN.pdf` | Full protocol spec v2.2: frame format, all command/reply definitions, CAN parameters, command sample with CRC values, NATO port pinout. |
| `docs/external/dji-r-sdk/docs/DJI_R_SDK_Protocol_and_User_Interface_EN_v2.5.pdf` | Protocol spec v2.5: same as v2.2 plus focus motor control (CmdID 0x12), narrowed angle ranges (roll +/-30, pitch -56/+146), CmdSet/CmdID added to reply frames. |
| `docs/research/dji-gimbal-software-control/report.md` | Survey of all open-source DJI gimbal projects. Lists DJIR_SDK (C++), dji_rs3pro_ros_controller (C++/ROS), DJI-Ronin-RS2-Log-and-Replay (Python) as reference implementations. |
| `docs/project/adr-001-microcontroller.md` | ESP32-S3 selection rationale. TWAI peripheral details, dual-core architecture for CAN on Core 1, GPIO budget, TJA1051 transceiver requirement. |
