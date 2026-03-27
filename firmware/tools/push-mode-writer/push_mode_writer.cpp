// TLV writer test — toggle push pan and push tilt via CAN bus.
// Writes TLV 0x14 (push pan) and 0x15 (push tilt) using CmdID 0x0C.
// Sequence: enable push pan → wait 10s → disable → enable push tilt → wait 10s → disable

#include <Arduino.h>
#include <driver/twai.h>
#include <cstring>
#include "dji_protocol.h"
#include "dji_crc.h"
#include "dji_types.h"

#define CAN_TX_PIN GPIO_NUM_5
#define CAN_RX_PIN GPIO_NUM_6

bool sendSdkFrame(const uint8_t* frame, size_t len) {
    size_t offset = 0;
    while (offset < len) {
        twai_message_t msg;
        msg.identifier = dji::CAN_TX_ID;
        msg.extd = 0;
        msg.rtr = 0;
        msg.data_length_code = (len - offset > 8) ? 8 : (len - offset);
        memcpy(msg.data, &frame[offset], msg.data_length_code);
        if (twai_transmit(&msg, pdMS_TO_TICKS(100)) != ESP_OK) return false;
        offset += msg.data_length_code;
    }
    return true;
}

dji::SequenceCounter seq;
uint8_t tx_buf[dji::MAX_FRAME_SIZE];

void sendCmd(uint8_t cmd_set, uint8_t cmd_id, const uint8_t* payload, size_t plen) {
    size_t flen = dji::buildFrame(dji::CMD_TYPE_CMD_REPLY_REQUIRED,
                                   cmd_set, cmd_id, payload, plen, seq.next(), tx_buf);
    sendSdkFrame(tx_buf, flen);
}

// Reassembly
uint8_t reassembly[256];
size_t reassembly_len = 0;
size_t reassembly_expected = 0;

bool receiveFrame(uint32_t timeout_ms) {
    uint32_t start = millis();
    reassembly_len = 0;
    reassembly_expected = 0;
    while (millis() - start < timeout_ms) {
        twai_message_t msg;
        if (twai_receive(&msg, pdMS_TO_TICKS(10)) == ESP_OK) {
            if (msg.identifier != dji::CAN_RX_ID) continue;
            for (uint8_t i = 0; i < msg.data_length_code && reassembly_len < sizeof(reassembly); i++)
                reassembly[reassembly_len++] = msg.data[i];
            if (reassembly_len >= 1 && reassembly[0] != 0xAA) { reassembly_len = 0; continue; }
            if (reassembly_expected == 0 && reassembly_len >= 3) {
                reassembly_expected = dji::extractFrameLength(reassembly);
                if (reassembly_expected == 0) { reassembly_len = 0; continue; }
            }
            if (reassembly_expected > 0 && reassembly_len >= reassembly_expected) {
                return dji::validateFrame(reassembly, reassembly_expected);
            }
        }
    }
    return false;
}

void sendEnablePush() {
    uint8_t payload[] = {0x01};
    sendCmd(0x0E, 0x07, payload, 1);
}

// Write a TLV using CmdSet=0x0E CmdID=0x0C (Set User Parameters)
// TLV format: ID(1) + Length(1) + Value(N)
void writeTlv(uint8_t id, uint8_t value) {
    uint8_t payload[] = {
        id,     // TLV ID
        0x01,   // Length = 1 byte
        value   // Value
    };
    sendCmd(0x0E, 0x0C, payload, 3);

    Serial.printf("  TX: write TLV 0x%02X = 0x%02X ... ", id, value);

    if (receiveFrame(500)) {
        uint8_t cmd_type, cmd_set, cmd_id;
        uint16_t frame_seq;
        const uint8_t* pl;
        size_t pl_len;
        if (dji::parseFrame(reassembly, reassembly_expected, cmd_type, frame_seq, cmd_set, cmd_id, pl, pl_len)) {
            if (pl_len >= 1) {
                Serial.printf("rc=%d", pl[0]);
                if (pl_len > 1) {
                    Serial.printf(" data:");
                    for (size_t i = 1; i < pl_len; i++) Serial.printf(" %02X", pl[i]);
                }
            }
        }
        Serial.println();
    } else {
        Serial.println("no response");
    }
}

// Read a TLV using CmdID=0x0B
void readTlv(uint8_t id) {
    uint8_t payload[] = {id};
    sendCmd(0x0E, 0x0B, payload, 1);

    if (receiveFrame(500)) {
        uint8_t cmd_type, cmd_set, cmd_id;
        uint16_t frame_seq;
        const uint8_t* pl;
        size_t pl_len;
        if (dji::parseFrame(reassembly, reassembly_expected, cmd_type, frame_seq, cmd_set, cmd_id, pl, pl_len)) {
            if (pl_len >= 1) {
                Serial.printf("  RX: read TLV 0x%02X rc=%d", id, pl[0]);
                if (pl[0] == 0 && pl_len > 1) {
                    Serial.printf(" data:");
                    for (size_t i = 1; i < pl_len; i++) Serial.printf(" %02X", pl[i]);
                }
                Serial.println();
            }
        }
    } else {
        Serial.printf("  RX: read TLV 0x%02X no response\n", id);
    }
}

bool connected = false;
uint32_t last_push_ms = 0;
int step = 0;
uint32_t step_time = 0;

void setup() {
    Serial.begin(115200);
    delay(2000);
    Serial.println("\n=== DJI RS 5 Push Mode Write Test ===");
    Serial.println("Make sure push pan and push tilt are both OFF in the Ronin app.\n");

    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(CAN_TX_PIN, CAN_RX_PIN, TWAI_MODE_NORMAL);
    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_1MBITS();
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();
    twai_driver_install(&g_config, &t_config, &f_config);
    twai_start();

    sendEnablePush();
    last_push_ms = millis();
}

void loop() {
    uint32_t now = millis();

    if (now - last_push_ms >= 5000) {
        sendEnablePush();
        last_push_ms = now;
    }

    // Wait for connection
    if (!connected) {
        if (receiveFrame(1000)) {
            if (reassembly[12] == 0x0E && reassembly[13] == 0x08) {
                connected = true;
                Serial.println("Connected to gimbal.\n");
                step_time = now;
            }
        }
        return;
    }

    // Step through the test
    if (step == 0 && now - step_time >= 2000) {
        Serial.println("--- Step 1: Read current push state ---");
        readTlv(0x14);
        delay(100);
        readTlv(0x15);
        step = 1;
        step_time = now;
    }
    else if (step == 1 && now - step_time >= 2000) {
        Serial.println("\n--- Step 2: ENABLE push pan (TLV 0x14 = 1) ---");
        Serial.println("    Try pushing the gimbal sideways with your hand...\n");
        writeTlv(0x14, 0x01);
        step = 2;
        step_time = now;
    }
    else if (step == 2 && now - step_time >= 10000) {
        Serial.println("\n--- Step 3: DISABLE push pan (TLV 0x14 = 0) ---\n");
        writeTlv(0x14, 0x00);
        step = 3;
        step_time = now;
    }
    else if (step == 3 && now - step_time >= 2000) {
        Serial.println("\n--- Step 4: ENABLE push tilt (TLV 0x15 = 1) ---");
        Serial.println("    Try tilting the gimbal up/down with your hand...\n");
        writeTlv(0x15, 0x01);
        step = 4;
        step_time = now;
    }
    else if (step == 4 && now - step_time >= 10000) {
        Serial.println("\n--- Step 5: DISABLE push tilt (TLV 0x15 = 0) ---\n");
        writeTlv(0x15, 0x00);
        step = 5;
        step_time = now;
    }
    else if (step == 5 && now - step_time >= 2000) {
        Serial.println("\n--- Step 6: Verify both OFF ---");
        readTlv(0x14);
        delay(100);
        readTlv(0x15);
        Serial.println("\n=== Test complete ===");
        step = 6;
    }
}
