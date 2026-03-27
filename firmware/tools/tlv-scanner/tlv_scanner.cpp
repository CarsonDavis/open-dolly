// CAN test — Read user parameter TLV IDs 0x00-0x30 from the gimbal.
// Run once with push mode OFF, note the output.
// Toggle push mode ON in the Ronin app, reset the ESP, compare output.

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

void sendCmd(uint8_t cmd_set, uint8_t cmd_id, const uint8_t* payload, size_t len) {
    size_t flen = dji::buildFrame(dji::CMD_TYPE_CMD_REPLY_REQUIRED,
                                   cmd_set, cmd_id, payload, len, seq.next(), tx_buf);
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

// Read a single TLV ID using CmdSet=0x0E CmdID=0x0B (Obtain User Parameters)
void readTlvId(uint8_t id) {
    uint8_t payload[] = {id};
    sendCmd(0x0E, 0x0B, payload, 1);

    if (receiveFrame(500)) {
        uint8_t cmd_type, cmd_set, cmd_id;
        uint16_t frame_seq;
        const uint8_t* pl;
        size_t pl_len;
        if (dji::parseFrame(reassembly, reassembly_expected, cmd_type, frame_seq, cmd_set, cmd_id, pl, pl_len)) {
            if (pl_len >= 1) {
                uint8_t return_code = pl[0];
                Serial.printf("  TLV 0x%02X: rc=%d", id, return_code);
                if (return_code == 0 && pl_len > 1) {
                    Serial.printf(" data(%d):", (int)(pl_len - 1));
                    for (size_t i = 1; i < pl_len; i++) {
                        Serial.printf(" %02X", pl[i]);
                    }
                }
                Serial.println();
            }
        }
    } else {
        Serial.printf("  TLV 0x%02X: no response\n", id);
    }
}

bool connected = false;
bool scan_done = false;
uint32_t last_push_ms = 0;

void setup() {
    Serial.begin(115200);
    delay(2000);
    Serial.println("\n=== DJI RS 5 TLV Parameter Scanner ===\n");

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

    // Wait for first telemetry to confirm connection
    if (!connected) {
        if (receiveFrame(1000)) {
            uint8_t cs = reassembly[12];
            uint8_t ci = reassembly[13];
            if (cs == 0x0E && ci == 0x08) {
                connected = true;
                Serial.println("Connected to gimbal.\n");
                Serial.println("Scanning TLV IDs 0x00 - 0x3F...\n");
            }
        }
        if (!connected && now - last_push_ms >= 3000) {
            sendEnablePush();
            last_push_ms = now;
        }
        return;
    }

    // Scan TLV IDs
    if (!scan_done) {
        for (uint8_t id = 0x00; id <= 0x3F; id++) {
            readTlvId(id);
            delay(100);  // give gimbal time between requests
        }
        Serial.println("\n=== Scan complete ===");
        Serial.println("Toggle push mode in the Ronin app and reset the ESP to scan again.\n");
        scan_done = true;
    }
}
