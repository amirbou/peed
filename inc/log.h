#include <stdio.h>

#define _LOG(fmt, ...) do { \
    printf("%s:%d:%s() " fmt "\n", __FILE__, __LINE__, __func__, ##__VA_ARGS__); \
} while (0)

#define PERROR(msg) do { perror(msg); } while(0)
#define ERROR(fmt, ...) _LOG("E: " fmt, ##__VA_ARGS__)
#define INFO(fmt, ...) _LOG("I: " fmt, ##__VA_ARGS__)

#ifdef _DEBUG
#define DEBUG(fmt, ...) _LOG("D: " fmt, ##__VA_ARGS__)
#else
#define DEBUG(...)
#endif