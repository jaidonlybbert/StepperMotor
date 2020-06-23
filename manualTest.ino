/* 
   Stepper motor controller
   for 28BYJ-48 packaged with
   elegoo uno starter kit
*/

#include <Arduino.h>

const int stepsPerRevolution = (32 * 64);

class Stepper_28byj {
  public:
    int pin0, pin1, pin2, pin3;
    long stepDelay = 0;
    
    Stepper_28byj (int pin0, int pin1, int pin2, int pin3) {
      this->pin0 = pin0;
      this->pin1 = pin1;
      this->pin2 = pin2;
      this->pin3 = pin3;
      
      pinMode(pin0, OUTPUT);
      pinMode(pin1, OUTPUT);
      pinMode(pin2, OUTPUT);
      pinMode(pin3, OUTPUT);
    }
    
    void stepLoop (int maxStep) {
      int i = 0;
      long lastStep = 0;
      long now = millis();
      
      if (maxStep > 0) {
        while (i < maxStep) {
          now = millis();
          if ((now - lastStep) >= this->stepDelay) {
            lastStep = now;
            halfStep(i % 8);
            i++;
          }
        }
      } else if (maxStep < 0) {
        i = abs(maxStep);
        while (i > 0) {
          now = millis();
          if ((now - lastStep) >= this->stepDelay) {
            lastStep = now;
            halfStep(i % 8);
            i--;
          }
        }
      }
    }
    
    void setRpm (int rpm) {
      this->stepDelay = (((60L * 1000L) / rpm) / stepsPerRevolution);
      Serial.println(this->stepDelay);
    }
    
    void stepOnce (int currentStep) {
      switch (currentStep) {
        case 0:
          digitalWrite(this->pin0, HIGH);
          digitalWrite(this->pin1, LOW);
          digitalWrite(this->pin2, LOW);
          digitalWrite(this->pin3, LOW);
          break;
        case 1:
          digitalWrite(this->pin0, LOW);
          digitalWrite(this->pin1, HIGH);
          digitalWrite(this->pin2, LOW);
          digitalWrite(this->pin3, LOW);
          break;
        case 2:
          digitalWrite(this->pin0, LOW);
          digitalWrite(this->pin1, LOW);
          digitalWrite(this->pin2, HIGH);
          digitalWrite(this->pin3, LOW);
          break;
        case 3:
          digitalWrite(this->pin0, LOW);
          digitalWrite(this->pin1, LOW);
          digitalWrite(this->pin2, LOW);
          digitalWrite(this->pin3, HIGH);
          break;
      }
    }
    
    void halfStep (int currentStep) {
      switch (currentStep) {
        case 0:
          digitalWrite(this->pin0, HIGH);
          digitalWrite(this->pin1, LOW);
          digitalWrite(this->pin2, LOW);
          digitalWrite(this->pin3, LOW);
          break;
        case 1:
          digitalWrite(this->pin0, HIGH);
          digitalWrite(this->pin1, HIGH);
          digitalWrite(this->pin2, LOW);
          digitalWrite(this->pin3, LOW);
          break;
        case 2:
          digitalWrite(this->pin0, LOW);
          digitalWrite(this->pin1, HIGH);
          digitalWrite(this->pin2, LOW);
          digitalWrite(this->pin3, LOW);
          break;
        case 3:
          digitalWrite(this->pin0, LOW);
          digitalWrite(this->pin1, HIGH);
          digitalWrite(this->pin2, HIGH);
          digitalWrite(this->pin3, LOW);
          break;
        case 4:
          digitalWrite(this->pin0, LOW);
          digitalWrite(this->pin1, LOW);
          digitalWrite(this->pin2, HIGH);
          digitalWrite(this->pin3, LOW);
          break;
        case 5:
          digitalWrite(this->pin0, LOW);
          digitalWrite(this->pin1, LOW);
          digitalWrite(this->pin2, HIGH);
          digitalWrite(this->pin3, HIGH);
          break;
        case 6:
          digitalWrite(this->pin0, LOW);
          digitalWrite(this->pin1, LOW);
          digitalWrite(this->pin2, LOW);
          digitalWrite(this->pin3, HIGH);
          break;
        case 7:
          digitalWrite(this->pin0, HIGH);
          digitalWrite(this->pin1, LOW);
          digitalWrite(this->pin2, LOW);
          digitalWrite(this->pin3, HIGH);
          break;
      }
    }
};

Stepper_28byj stepper(8, 9, 10, 11);

void setup() {
  stepper.setRpm(6);
  Serial.begin(9600);
}

void loop() {
  Serial.println("begin");
  stepper.stepLoop(stepsPerRevolution);
  Serial.println("finish");
  delay(500);
  
  stepper.stepLoop(-stepsPerRevolution);
  delay(500);
}
        
