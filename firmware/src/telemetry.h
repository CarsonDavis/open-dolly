#pragma once

#include <cstdint>

#ifndef NATIVE_TEST

/// FreeRTOS task: reads TelemetryPoint structs from the telemetry queue,
/// serializes them to JSON, and broadcasts to all connected WebSocket clients.
///
/// @param param  unused (pass nullptr)
void telemetryBroadcastTask(void* param);

#endif // NATIVE_TEST
