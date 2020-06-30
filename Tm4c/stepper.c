#include <stdint.h>
#include <stdlib.h>
#include "stepper.h"

uint16_t cStep = 0; // Current step number; i.e. address for absolute positioning; should be "home" position

void PortA_Init(void) {             // initialize stepper data pins
  *SYSCTL_RCGCGPIO_R |= 0x01;       // activate clock on Port A
  *GPIO_PORTA_DIR_R |= 0x3C;        // make PA5-2 pins output
  *GPIO_PORTA_DEN_R |= 0x3C;        // enable digital I/O on PA5-2
}

void PortF_Init(void) {               // initialize sw2
    *SYSCTL_RCGCGPIO_R |= 0x20;       // activate clock on Port F
    *GPIO_PORTF_DIR_R &= ~0x10;       // make PF4 an input pin
    *GPIO_PORTF_DEN_R |= 0x10;        // enable digital i/o on PF4
    *GPIO_PORTF_PUR_R |= 0x10;        // enable PUR on sw2
}

// Define the states in the FSM
State_t stepFsm[4] = {
    {0, 0x04, {1, 3} },
    {1, 0x08, {2, 0} },
    {2, 0x10, {3, 1} },
    {3, 0x20, {0, 2} }
};

// Quick and dirty busy-loop delay
// This will need updating to a more precise delay
void delayT(int maxCount) {
    for (volatile int j = 0; j < maxCount; j++) {continue;}
}

// Step the motor once
// Direction map: 0x00 = CW; 0x01 = CCW; 
void stepOnce(uint8_t direction, uint8_t *cState) {
    *cState = stepFsm[*cState].next[direction]; // Move to next state defined by direction
    *GPIO_PORTA_DATA_R = stepFsm[*cState].out;  // Output motor data defined in new state
    
    // Update cStep--this algorithm can be improved
    if (direction == 0x01) { 
        if (cStep-1 < 0) {         // wrap around
            cStep = MAX_STEP;
        } else {
            cStep--; 
        }
    } else { 
        if (cStep+1 == MAX_STEP) {  // wrap around
            cStep = 0;          
        } else { 
            cStep++; 
        }
        cStep++; 
    }                         
}

// Debounce a button input
// This function must be called at a high frequency to work
// Inverts the "input" variable after the button is released
void debounce(uint8_t* input, uint8_t* flag) {
    if (*GPIO_PORTF_DATA_R == 16) {             // switch unpressed
            delayT(1000);
            if (*GPIO_PORTF_DATA_R == 16 && *flag == 0) {
                *input = (~(*input))&0x01;      // invert input
                *flag = 1;
            }
    } else {
        *flag = 0;
    }
}

// Steps a number of times uninterrupted
// No stop condition
void rotate(uint8_t input, uint8_t* cState) {
    for (int i = 0; i < (64 * 32 / 5); i++) { // There are 64 * 32 steps per rotation
        stepOnce(input, cState);
        delayT(10000);
    }
}


// Debounce limit switch
uint8_t limitDebounce(uint8_t* data) {
    if( ((*data)&0x01) == 0x00) {
        delayT(1000);
        if( ((*data)&0x01) == 0x00) {
            return 0x00;
        }
        return 0x01;
    } else {
        return 0x01;
    }
}

// Homing Mode
void homingMode(uint8_t* data, uint8_t* mode, uint8_t* cState) {// data = direction & limitSw
    while(*mode == 0 && limitDebounce(data) == 0x01) {          // while the mode is unchanged and the switch is unpressed
        stepOnce(*data>>4, cState);                             // continuously step in the specified direction
    }

    cStep = 0;                                                   // reset motor address
}

// Absolute Positioning Mode 1
// Turns motor to specific step number CW from "home" position using shortest path
// This works for applications with full 360 range of motion
void absPosMode_360(uint8_t data, uint8_t* cState) {
    
    uint8_t direction = 0x00;

    // Determine shortest path from cStep to "data" step
    // This algorithm can definitely be improved
    if (data > cStep) {
        if ((data - cStep) < (MAX_STEP / 2)) {
            direction = DIRECTION_CW;
        } else {
            direction = DIRECTION_CCW;
        }
    } else {
        if ((cStep - data) < (MAX_STEP / 2)) {
            direction = DIRECTION_CCW;
        } else {
            direction = DIRECTION_CW;
        }
    }

    // Continuously stepOnce in determined direction until cStep=="data"
    while(cStep != data) {
        stepOnce(direction, cState);
        delayT(10000);
    }
}


// Absolute Positioning Mode 2
// Turns motor to specific step number CW from "home" position across "pie slice" of rotation
// This works for motors with limited range of motion due to an obstruction
void absPosMode_Slice(uint8_t data, uint8_t* cState) {
    
    uint8_t direction = 0x00;

    // Determine shortest path from cStep to "data" step
    // This algorithm can definitely be improved
    if (data > cStep) {
        direction = DIRECTION_CW;
    } else {
        direction = DIRECTION_CCW;
    }

    // Continuously stepOnce in determined direction until cStep=="data"
    while(cStep != data) {
        stepOnce(direction, cState);
        delayT(10000);
    }
}

// Relative Positioning Mode
// Turns motor a specified number of steps in specified direction
// No stop condition
void relPosMode(uint8_t direction, uint8_t* cState, uint8_t numSteps) {
    for (int i = 0; i < numSteps; i++) {            
        stepOnce(direction, cState);
        delayT(10000);
    }
}

int main() {
    PortA_Init(); // initialize stepper data output port
    PortF_Init(); // initialize switch input port

    uint8_t cState = 0x00; // track current motor state
    uint8_t input = 0x01;  // store input signal (sw)

    while(1) {
        input = *GPIO_PORTF_DATA_R>>4;
        delayT(10000);
        stepOnce(input, &cState);
    }

    return 0;
}