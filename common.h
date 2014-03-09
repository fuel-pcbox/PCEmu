#ifndef COMMON_H
#define COMMON_H

#include <cstdio>
#include <cstdint>
#include <vector>

typedef uint8_t u8;
typedef int8_t s8;
typedef uint16_t u16;
typedef int16_t s16;
typedef uint32_t u32;
typedef int32_t s32;

#define log(...) printf("%s:%s %s", __FILE__, __LINE__, __VA_ARGS__)

#endif