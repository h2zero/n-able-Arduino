/*
 * FreeRTOS task demo
 * Shows how to blink an LED in it's own task.
 *
 * Note the use of the build_opt.h file to reduce the stack size of the main task.
 * Since the loop is empty the main task stack does not need as much stack space.
 */

#ifndef LED_BUILTIN
#define LED_BUILTIN 9
#endif

void BlinkTask( void *pvParameters );

void setup() {
    Serial.begin(115200);

    xTaskCreate(
    BlinkTask,  // The task function to run
    "blnk",     // name of the task
    128,        // Size of the task stack in words (32 bit)
    NULL,       // Pointer to data to pass to the task function or NULL.
    2,          // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    NULL);      // Pointer to a TaskHandle_t variable to store the task handle, or NULL.

  // The task is started immediately by the call to xTaskCreate.
}

void loop() {
    // Empty, we are using our own task.
}

// This is our blink task.
void BlinkTask(void *pvParameters) {
    (void) pvParameters;

    // initialize digital LED_BUILTIN as an output.
    pinMode(LED_BUILTIN, OUTPUT);

    for (;;) { // A Task shall never return or exit.
        digitalToggle(LED_BUILTIN);
        Serial.printf("Free heap: %u\n", RTOS.getFreeHeap());
        delay(500);
    }
}
