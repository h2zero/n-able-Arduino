#include "Arduino.h"
#include "printf.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#ifdef CONFIG_PRINTF_BUFFER_SIZE
#if CONFIG_PRINTF_BUFFER_SIZE < 1
#error "CONFIG_PRINTF_BUFFER_SIZE must be at least 1"
#endif
#define PRINTF_BUFFER_SIZE CONFIG_PRINTF_BUFFER_SIZE - 1 // reserve 1 byte for potential carriage return
#else
#ifdef NRF51
#define PRINTF_BUFFER_SIZE 1 // NRF51 has very limited RAM, no buffer
#else
#define PRINTF_BUFFER_SIZE 128 - 1 // Default to half the CDC write buffer to minimize task waits - 1 for carriage return if needed
#endif
#endif

// Structure to hold buffer info for each task
struct PrintfBuffer
{
    char buffer[PRINTF_BUFFER_SIZE + 1]; // allocate 1 more byte to insert carriage return if needed
    size_t index;
};

// FreeRTOS task notification index for buffer management
#ifdef CONFIG_PRINTF_BUFFER_INDEX
#define PRINTF_BUFFER_INDEX CONFIG_PRINTF_BUFFER_INDEX
#else
#define PRINTF_BUFFER_INDEX 0
#endif

extern "C"
{
    // Helper function to get or create buffer for current task
    static PrintfBuffer *getPrintfBuffer(void)
    {
#if (configNUM_THREAD_LOCAL_STORAGE_POINTERS)
        if (PRINTF_BUFFER_SIZE == 1)
        {
            // No buffer, just output single bytes
            return nullptr;
        }

        PrintfBuffer *pbuf = (PrintfBuffer *)pvTaskGetThreadLocalStoragePointer(NULL, PRINTF_BUFFER_INDEX);

        if (!pbuf)
        {
            pbuf = (PrintfBuffer *)malloc(sizeof(PrintfBuffer));
            if (pbuf)
            {
                pbuf->index = 0;
                vTaskSetThreadLocalStoragePointer(NULL, PRINTF_BUFFER_INDEX, (void *)pbuf);
            }
        }

        return pbuf;
#else
        return nullptr;
#endif
    }

    // Flush buffer to Serial
    static void flushPrintfBuffer(PrintfBuffer *pbuf)
    {
        if (pbuf && pbuf->index > 0)
        {
            Serial.write((const uint8_t *)pbuf->buffer, pbuf->index);
            pbuf->index = 0;
        }
#ifdef USB_CDC_DEFAULT_SERIAL
        Serial.flush();
#endif
    }

    void _putchar(const char c)
    {
        PrintfBuffer *pbuf = getPrintfBuffer();

        if (pbuf == nullptr)
        {
            // Fallback if malloc failed or buffer size is 1 - write directly
            if (c == '\n')
            {
                Serial.write('\r');
            }
            Serial.write(c);
#ifdef USB_CDC_DEFAULT_SERIAL
            Serial.flush();
#endif
            return;
        }

        // Handle newline by adding CR before LF
        if (c == '\n' && (pbuf->index == 0 || pbuf->buffer[pbuf->index - 1] != '\r'))
        {
            // 1 extra byte was allocated for this case, so it won't overflow the buffer
            pbuf->buffer[pbuf->index++] = '\r';
        }

        // Add character to buffer
        if (pbuf->index < PRINTF_BUFFER_SIZE)
        {
            pbuf->buffer[pbuf->index++] = c;
        }

        // Flush if buffer full or newline detected
        if (pbuf->index >= PRINTF_BUFFER_SIZE || c == '\n')
        {
            flushPrintfBuffer(pbuf);
        }
    }

    int _write(int fd, const void *buf, size_t count)
    {
        (void)fd;
        return (int)Serial.write((const uint8_t *)buf, count);
    }

} // extern "C"
