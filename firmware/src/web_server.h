#pragma once

#ifndef NATIVE_TEST

#include <ESPAsyncWebServer.h>
#include <AsyncWebSocket.h>

extern AsyncWebServer server;
extern AsyncWebSocket ws;
extern QueueHandle_t cmd_queue;
extern QueueHandle_t telemetry_queue;

void setupWebServer();
void sendStateEvent(AsyncWebSocketClient* client);
void broadcastStateEvent();
void sendError(AsyncWebSocketClient* client, const char* code, const char* detail);

#else

// Native test stubs
inline void broadcastStateEvent() {}

#endif // NATIVE_TEST
