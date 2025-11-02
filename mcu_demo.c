#include <stdint.h>

#define RAM_START 0x20000000
#define RAM_SIZE  0x5000
#define STACK_TOP (RAM_START + RAM_SIZE)

volatile uint32_t *ptr = (uint32_t *)RAM_START;

void delay(volatile int count) {
    while(count--) __asm__("nop");
}

// main loop
volatile int gval = 0;
void main_loop(void) {
    uint32_t val = 0;
    while(1) {
        gval++;
        val++;
        *ptr = val;
        delay(1000);
    }
}

// Reset Handler
void Reset_Handler(void) {
    main_loop();
}

// Vector Table
__attribute__ ((section(".isr_vector")))
void (* const vector_table[])(void) = {
    (void (*)(void))STACK_TOP, 
    Reset_Handler              
};
