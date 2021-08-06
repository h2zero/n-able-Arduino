#include "Arduino.h"
#include "printf.h"

extern "C" void _putchar(const char c)
{
    if(c == '\n') {
        Serial.write('\r');
    }
    Serial.write(c);
}
