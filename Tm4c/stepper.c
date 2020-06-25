#include <stdint.h>

uint32_t *SYSCTL_RCGCGPIO_R = (uint32_t*) (0x400FE000 + 0x608); // Run-mode clock gating control
uint32_t *GPIO_PORTA_DIR_R = (uint32_t*) (0x40004000 + 0x400); // Port A direction
uint32_t *GPIO_PORTA_DEN_R = (uint32_t*) (0x40004000 + 0x51C); // Port A Digital Enable
uint32_t *GPIO_PORTA_DATA_R = (uint32_t*) (0x40004000 + 0xF0); // Port A Data for Pins PA2-PA5

uint32_t *GPIO_PORTF_DIR_R = (uint32_t*) (0x40025000 + 0x400); // Port F direction
uint32_t *GPIO_PORTF_DEN_R = (uint32_t*) (0x40025000 + 0x51C); // Port F Digital Enable
uint32_t *GPIO_PORTF_DATA_R = (uint32_t*) (0x40025000 + 0x40); // Port F Data for Pin PF0 (sw2)
uint32_t *GPIO_PORTF_PUR_R = (uint32_t*) (0x40025000 + 0x510);

void PortA_Init(void){ 
  volatile uint32_t delay;
  *SYSCTL_RCGCGPIO_R |= 0x01;          // activate Port A
  delay = SYSCTL_RCGCGPIO_R;          // allow time for clock to stabilize
  *GPIO_PORTA_DIR_R |= 0x3C;       // make PA5-2 out
  *GPIO_PORTA_DEN_R |= 0x3C;        // enable digital I/O on PE1-0
}

void PortF_Init(void) {
    volatile uint32_t delay;
    *SYSCTL_RCGCGPIO_R |= 0x20;       // activate Port F
    delay = SYSCTL_RCGCGPIO_R;        // allow time for clock to stabilize
    *GPIO_PORTF_DIR_R &= ~0x10;       // make PF4 in
    *GPIO_PORTF_DEN_R |= 0x10;        // enable digital i/o on PF0
    *GPIO_PORTF_PUR_R |= 0x10;       // enable PUR on sw2
}

struct State {
    uint8_t out;
    uint8_t next[2];
};

typedef const struct State State_t;

State_t Fsm[4] = {
    {0x04, {1, 3} },
    {0x08, {2, 0} },
    {0x10, {3, 1} },
    {0x20, {0, 2} }
};

void delayT(int maxCount) {
    for (volatile int j = 0; j < maxCount; j++) {continue;}
}

int main() {
    PortA_Init();
    PortF_Init();

    uint8_t cState = 0;
    uint8_t input = 0x01;
    uint8_t flag = 0x01;
    while(1) {
        *GPIO_PORTA_DATA_R = Fsm[cState].out;

        // Debounce
        if (*GPIO_PORTF_DATA_R == 16) { // switch unpressed
            delayT(1000);
            if (*GPIO_PORTF_DATA_R == 16 && flag == 0) {
                input = ~input&0x01;
                flag = 1;
            }
        } else {
            flag = 0;
        }

        delayT(10000);
        cState = Fsm[cState].next[input];
    }

    return 0;
}