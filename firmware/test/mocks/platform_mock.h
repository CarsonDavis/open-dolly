#pragma once

// Mock platform layer for native testing.
// Provides stubs for ESP32 / Arduino / FreeRTOS APIs so firmware
// logic can be unit-tested on a host machine.

#ifdef NATIVE_TEST

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cstdio>

// --- PSRAM ---
#define ps_malloc(size) malloc(size)
#define ps_calloc(n, size) calloc(n, size)

// --- FreeRTOS stubs ---
typedef void* SemaphoreHandle_t;
typedef void* QueueHandle_t;
typedef int BaseType_t;
typedef uint32_t TickType_t;

#define pdTRUE 1
#define pdFALSE 0
#define pdMS_TO_TICKS(ms) (ms)
#define portMAX_DELAY 0xFFFFFFFF

inline SemaphoreHandle_t xSemaphoreCreateMutex() { return (void*)1; }
inline BaseType_t xSemaphoreTake(SemaphoreHandle_t, TickType_t) { return pdTRUE; }
inline BaseType_t xSemaphoreGive(SemaphoreHandle_t) { return pdTRUE; }

inline QueueHandle_t xQueueCreate(int, size_t) { return (void*)1; }
inline BaseType_t xQueueSend(QueueHandle_t, const void*, TickType_t) { return pdTRUE; }
inline BaseType_t xQueueReceive(QueueHandle_t, void*, TickType_t) { return pdFALSE; }

inline void vTaskDelay(TickType_t) {}

// --- Arduino stubs ---
static unsigned long _mock_millis_value = 0;

inline unsigned long millis() {
    return _mock_millis_value;
}

// Allow tests to advance mock time
inline void mock_set_millis(unsigned long ms) {
    _mock_millis_value = ms;
}

// Serial stub
struct MockSerial {
    template<typename... Args>
    void printf(const char* fmt, Args... args) {
        ::printf(fmt, args...);
    }
    void println(const char* s) { ::printf("%s\n", s); }
    void print(const char* s) { ::printf("%s", s); }
    void begin(int) {}
};
extern MockSerial Serial;

// GPIO stubs
#define INPUT_PULLUP 0x05
#define HIGH 1
#define LOW 0
#define FALLING 2

inline void pinMode(int, int) {}
inline int digitalRead(int) { return HIGH; }
inline void attachInterrupt(int, void(*)(), int) {}
inline int digitalPinToInterrupt(int pin) { return pin; }

// esp_random stub
inline uint32_t esp_random() {
    return (uint32_t)rand();
}

// strlcpy for platforms that lack it
#ifndef __APPLE__
inline size_t strlcpy(char* dst, const char* src, size_t size) {
    size_t len = strlen(src);
    if (size > 0) {
        size_t copy = (len >= size) ? size - 1 : len;
        memcpy(dst, src, copy);
        dst[copy] = '\0';
    }
    return len;
}
#endif

#endif // NATIVE_TEST
