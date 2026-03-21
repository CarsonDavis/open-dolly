#pragma once
#include <cstdint>
#include <cstddef>

namespace dji {

/// CRC16 over the DJI R SDK frame header (bytes 0..9).
/// Init value 0x3AA3, poly 0x8005, reflected I/O, no XOR out.
uint16_t crc16(const uint8_t* data, size_t length);

/// CRC32 over the DJI R SDK frame body (all bytes except the trailing 4-byte CRC32).
/// Init value 0x00003AA3, poly 0x04C11DB7, reflected I/O, no XOR out.
uint32_t crc32(const uint8_t* data, size_t length);

}  // namespace dji
