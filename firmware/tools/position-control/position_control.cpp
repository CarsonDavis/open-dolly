// Sends absolute position commands to move the gimbal.
// Cycles through: pitch up, pitch down, roll right, roll left, center.
// Uses the dji_can library for correct CRC and frame building.

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

void sendPosition(float yaw_deg, float pitch_deg, float roll_deg, float duration_sec) {
    int16_t yaw_raw   = (int16_t)(yaw_deg * 10.0f);
    int16_t roll_raw  = (int16_t)(roll_deg * 10.0f);
    int16_t pitch_raw = (int16_t)(pitch_deg * 10.0f);
    uint8_t time_action = (uint8_t)(duration_sec * 10.0f);

    uint8_t payload[8] = {
        (uint8_t)(yaw_raw & 0xFF), (uint8_t)((yaw_raw >> 8) & 0xFF),
        (uint8_t)(roll_raw & 0xFF), (uint8_t)((roll_raw >> 8) & 0xFF),
        (uint8_t)(pitch_raw & 0xFF), (uint8_t)((pitch_raw >> 8) & 0xFF),
        0x01,  // absolute, all axes valid
        time_action
    };

    size_t len = dji::buildFrame(dji::CMD_TYPE_CMD_REPLY_REQUIRED,
                                  dji::CMD_SET_GIMBAL, dji::CMD_ID_POSITION_CONTROL,
                                  payload, 8, seq.next(), tx_buf);
    Serial.printf("TX position: yaw=%.1f pitch=%.1f roll=%.1f dur=%.1fs\n",
                  yaw_deg, pitch_deg, roll_deg, duration_sec);
    sendSdkFrame(tx_buf, len);
}

// Telemetry
uint8_t reassembly[256];
size_t reassembly_len = 0;
size_t reassembly_expected = 0;
bool got_telemetry = false;
uint32_t last_push_ms = 0;
uint32_t last_move_ms = 0;
int move_step = 0;

struct Move { float yaw, pitch, roll; const char* desc; };
Move moves[] = {
    { 45.0,   0.0,  0.0, "Pan right 45"},
    {-45.0,   0.0,  0.0, "Pan left 45"},
    {  0.0,  30.0,  0.0, "Pitch up 30"},
    {  0.0, -20.0,  0.0, "Pitch down 20"},
    {  0.0,   0.0, 15.0, "Roll right 15"},
    {  0.0,   0.0,-15.0, "Roll left 15"},
    {  0.0,   0.0,  0.0, "Center"},
};
const int NUM_MOVES = sizeof(moves) / sizeof(moves[0]);

void setup() {
    Serial.begin(115200);
    delay(2000);
    Serial.println("\n=== DJI RS 5 Position Control Test ===\n");

    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(CAN_TX_PIN, CAN_RX_PIN, TWAI_MODE_NORMAL);
    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_1MBITS();
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();
    twai_driver_install(&g_config, &t_config, &f_config);
    twai_start();

    uint8_t p[] = {0x01};
    sendCmd(0x0E, 0x07, p, 1);
    last_push_ms = millis();
}

void loop() {
    uint32_t now = millis();

    if (now - last_push_ms >= 5000) {
        uint8_t p[] = {0x01};
        sendCmd(0x0E, 0x07, p, 1);
        last_push_ms = now;
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
                if (!got_telemetry) { got_telemetry = true; Serial.println("Telemetry active!\n"); }
                uint8_t cs = reassembly[12], ci = reassembly[13];
                if (cs == 0x0E && ci == 0x08) {
                    const uint8_t* d = &reassembly[14];
                    if (d[0] & 0x01) {
                        int16_t y = (int16_t)(d[1] | (d[2] << 8));
                        int16_t r = (int16_t)(d[3] | (d[4] << 8));
                        int16_t p = (int16_t)(d[5] | (d[6] << 8));
                        Serial.printf("    yaw=%6.1f roll=%6.1f pitch=%6.1f\n", y*0.1f, r*0.1f, p*0.1f);
                    }
                }
            }
            reassembly_len = 0;
            reassembly_expected = 0;
        }
    }

    if (got_telemetry && move_step < NUM_MOVES && (last_move_ms == 0 || now - last_move_ms >= 4000)) {
        Serial.printf("\n>>> Step %d: %s\n", move_step + 1, moves[move_step].desc);
        sendPosition(moves[move_step].yaw, moves[move_step].pitch, moves[move_step].roll, 2.0);
        move_step++;
        last_move_ms = now;
    }

    if (move_step >= NUM_MOVES && now - last_move_ms >= 4000) {
        Serial.println("\n=== All moves complete! ===");
        move_step = NUM_MOVES + 1;
    }
}
