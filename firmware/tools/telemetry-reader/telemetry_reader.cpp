// Sends enable-push and decodes live gimbal telemetry.
// Prints attitude angles (yaw, roll, pitch), limits, and stiffness.
// Move the gimbal by hand to see values change in real-time.

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

void sendEnablePush() {
    uint8_t payload[] = {0x01};
    size_t len = dji::buildFrame(dji::CMD_TYPE_CMD_REPLY_REQUIRED,
                                  dji::CMD_SET_GIMBAL, dji::CMD_ID_SET_PUSH,
                                  payload, 1, seq.next(), tx_buf);
    sendSdkFrame(tx_buf, len);
}

uint8_t reassembly[256];
size_t reassembly_len = 0;
size_t reassembly_expected = 0;
uint32_t last_push_ms = 0;
uint32_t frame_count = 0;

void parseTelemetry(const uint8_t* frame, size_t len) {
    if (len < dji::FRAME_OVERHEAD) return;
    uint8_t cmd_set = frame[12];
    uint8_t cmd_id = frame[13];

    if (cmd_set != 0x0E || cmd_id != 0x08) return;

    const uint8_t* data = &frame[14];
    size_t data_len = len - dji::FRAME_OVERHEAD;
    if (data_len < 13) return;

    uint8_t ctrl = data[0];
    if (!(ctrl & 0x01)) return;

    int16_t yaw  = (int16_t)(data[1]  | (data[2]  << 8));
    int16_t roll = (int16_t)(data[3]  | (data[4]  << 8));
    int16_t pit  = (int16_t)(data[5]  | (data[6]  << 8));
    int16_t yj   = (int16_t)(data[7]  | (data[8]  << 8));
    int16_t rj   = (int16_t)(data[9]  | (data[10] << 8));
    int16_t pj   = (int16_t)(data[11] | (data[12] << 8));

    frame_count++;
    Serial.printf("[#%lu] Att: yaw=%6.1f roll=%6.1f pitch=%6.1f | Jnt: yaw=%6.1f roll=%6.1f pitch=%6.1f",
        frame_count,
        yaw * 0.1f, roll * 0.1f, pit * 0.1f,
        yj * 0.1f, rj * 0.1f, pj * 0.1f);

    if ((ctrl & 0x02) && data_len >= 19) {
        Serial.printf(" | Lim: P[%d-%d] Y[%d-%d] R[%d-%d]",
            data[14], data[13], data[16], data[15], data[18], data[17]);
    }
    if ((ctrl & 0x04) && data_len >= 22) {
        Serial.printf(" | Stiff: P=%d Y=%d R=%d", data[19], data[20], data[21]);
    }
    Serial.println();
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n=== DJI RS 5 Telemetry Reader ===");
    Serial.println("Move the gimbal by hand to see angle values change.\n");

    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(CAN_TX_PIN, CAN_RX_PIN, TWAI_MODE_NORMAL);
    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_1MBITS();
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();
    twai_driver_install(&g_config, &t_config, &f_config);
    twai_start();

    sendEnablePush();
    last_push_ms = millis();
    Serial.println("Enable-push sent. Waiting for telemetry...\n");
}

void loop() {
    if (millis() - last_push_ms >= 5000) {
        sendEnablePush();
        last_push_ms = millis();
    }

    twai_message_t msg;
    while (twai_receive(&msg, pdMS_TO_TICKS(10)) == ESP_OK) {
        if (msg.identifier != dji::CAN_RX_ID) continue;
        for (uint8_t i = 0; i < msg.data_length_code && reassembly_len < sizeof(reassembly); i++)
            reassembly[reassembly_len++] = msg.data[i];
        if (reassembly_len >= 1 && reassembly[0] != 0xAA) { reassembly_len = 0; continue; }
        if (reassembly_expected == 0 && reassembly_len >= 3) {
            reassembly_expected = dji::extractFrameLength(reassembly);
            if (reassembly_expected == 0) { reassembly_len = 0; continue; }
        }
        if (reassembly_expected > 0 && reassembly_len >= reassembly_expected) {
            if (dji::validateFrame(reassembly, reassembly_expected)) {
                parseTelemetry(reassembly, reassembly_expected);
            }
            reassembly_len = 0;
            reassembly_expected = 0;
        }
    }
}
