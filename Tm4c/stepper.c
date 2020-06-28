#include <stdint.h>
#include "stepper.h"

void PortA_Init(void) { 
  volatile uint32_t delay;
  *SYSCTL_RCGCGPIO_R |= 0x01;       // activate Port A
  delay = *SYSCTL_RCGCGPIO_R;        // allow time for clock to stabilize
  *GPIO_PORTA_DIR_R |= 0x3C;        // make PA5-2 out
  *GPIO_PORTA_DEN_R |= 0x3C;        // enable digital I/O on PE1-0
}

void PortF_Init(void) {
    volatile uint32_t delay;
    *SYSCTL_RCGCGPIO_R |= 0x20;       // activate Port F
    delay = *SYSCTL_RCGCGPIO_R;        // allow time for clock to stabilize
    *GPIO_PORTF_DIR_R &= ~0x10;       // make PF4 in
    *GPIO_PORTF_DEN_R |= 0x10;        // enable digital i/o on PF0
    *GPIO_PORTF_PUR_R |= 0x10;        // enable PUR on sw2
}

struct State {
    uint8_t cState;
    uint8_t out;
    uint8_t next[2];
};

typedef const struct State State_t;

State_t stepFsm[4] = {
    {0, 0x04, {1, 3} },
    {1, 0x08, {2, 0} },
    {2, 0x10, {3, 1} },
    {3, 0x20, {0, 2} }
};

void delayT(int maxCount) {
    for (volatile int j = 0; j < maxCount; j++) {continue;}
}

uint8_t stepOnce(uint8_t direction, uint8_t cState) {
    cState = stepFsm[cState].next[direction];
    *GPIO_PORTA_DATA_R = stepFsm[cState].out;
    return cState;
}

void debounce(uint8_t* input, uint8_t* flag) {
    if (*GPIO_PORTF_DATA_R == 16) { // switch unpressed
            delayT(1000);
            if (*GPIO_PORTF_DATA_R == 16 && *flag == 0) {
                *input = (~(*input))&0x01; // invert input
                *flag = 1;
            }
    } else {
        *flag = 0;
    }
}

void rotate(uint8_t input, uint8_t* cState) {
    for (int i = 0; i < (64 * 32 / 5); i++) { // There are 64 * 32 steps per rotation
        *cState = stepOnce(input, *cState);
        delayT(1000);
    }
}

int main() {
    PortA_Init();
    PortF_Init();

    uint8_t* cState;
    *cState = 0;
    uint8_t* input;
    *input = 0x01;
    uint8_t* flag;
    *flag = 0x01;

    while(1) {
        debounce(input, flag);  // update input
        delayT(10000);          // wait
        rotate(*input, cState); // rotate 1/5 rotation
    }

    return 0;
}