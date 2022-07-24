#include "Arduino.h"
#include "printf.h"

extern "C"
{

void _putchar(const char c)
{
    if(c == '\n') {
        Serial.write('\r');
    }
    Serial.write(c);
}

int _write (int fd, const void *buf, size_t count)
{
  (void) fd;
  return (int)Serial.write((const uint8_t*)buf, count);
}

} //extern "C"
