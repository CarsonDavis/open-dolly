// Passive CAN bus listener — prints all raw CAN frames.
// No commands are sent. TWAI runs in listen-only mode.

#include <Arduino.h>
#include <driver/twai.h>

#define CAN_TX_PIN GPIO_NUM_5
#define CAN_RX_PIN GPIO_NUM_6

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n=== DJI RS 5 CAN Bus Listener ===");
    Serial.printf("CAN TX pin: %d, RX pin: %d\n", CAN_TX_PIN, CAN_RX_PIN);

    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(CAN_TX_PIN, CAN_RX_PIN, TWAI_MODE_LISTEN_ONLY);
    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_1MBITS();
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    esp_err_t err = twai_driver_install(&g_config, &t_config, &f_config);
    if (err != ESP_OK) {
        Serial.printf("TWAI install failed: %s\n", esp_err_to_name(err));
        return;
    }

    err = twai_start();
    if (err != ESP_OK) {
        Serial.printf("TWAI start failed: %s\n", esp_err_to_name(err));
        return;
    }

    Serial.println("TWAI started in listen-only mode. Waiting for CAN frames...\n");
}

void loop() {
    twai_message_t msg;
    esp_err_t err = twai_receive(&msg, pdMS_TO_TICKS(100));

    if (err == ESP_OK) {
        Serial.printf("ID: 0x%03X  DLC: %d  Data:", msg.identifier, msg.data_length_code);
        for (int i = 0; i < msg.data_length_code; i++) {
            Serial.printf(" %02X", msg.data[i]);
        }
        Serial.println();
    }
}
