#include <Arduino.h>

volatile int interruptCounter_0;
int perdNum;

const uint8_t PIN1=18, PIN2=19, PIN3=21;
hw_timer_t * timer0 = NULL;
hw_timer_t * timer1 = NULL;
hw_timer_t * timer2 = NULL; 
hw_timer_t * timer3 = NULL;
portMUX_TYPE timerMux0 = portMUX_INITIALIZER_UNLOCKED;
portMUX_TYPE timerMux1 = portMUX_INITIALIZER_UNLOCKED;
portMUX_TYPE timerMux2 = portMUX_INITIALIZER_UNLOCKED;
portMUX_TYPE timerMux3 = portMUX_INITIALIZER_UNLOCKED;

void IRAM_ATTR onTimer_0() {
  portENTER_CRITICAL_ISR(&timerMux0);
  interruptCounter_0=!interruptCounter_0;
  portEXIT_CRITICAL_ISR(&timerMux0);
}

void IRAM_ATTR onTimer_1() {
  portENTER_CRITICAL_ISR(&timerMux1);
  digitalWrite(PIN1,!digitalRead(PIN1));
  portEXIT_CRITICAL_ISR(&timerMux1);
}
void IRAM_ATTR onTimer_2() {
  portENTER_CRITICAL_ISR(&timerMux2);
  digitalWrite(PIN2,!digitalRead(PIN2));
  portEXIT_CRITICAL_ISR(&timerMux2);
}
void IRAM_ATTR onTimer_3() {
  portENTER_CRITICAL_ISR(&timerMux3);
  digitalWrite(PIN3,!digitalRead(PIN3));
  portEXIT_CRITICAL_ISR(&timerMux3);
}


void setup() {
  Serial.begin(115200);
  pinMode(PIN1,OUTPUT);
  pinMode(PIN2,OUTPUT);
  pinMode(PIN3,OUTPUT);

  timer0 = timerBegin(0, 80, true);
  timer1 = timerBegin(1, 80, true);
  timer2 = timerBegin(2, 80, true);
  timer3 = timerBegin(3, 80, true);

  timerAttachInterrupt(timer0, &onTimer_0, true);
  timerAttachInterrupt(timer1, &onTimer_1, true);
  timerAttachInterrupt(timer2, &onTimer_2, true);
  timerAttachInterrupt(timer3, &onTimer_3, true);

  timerAlarmWrite(timer0, 500000, true);
  timerAlarmWrite(timer1, 1100000, true);
  timerAlarmWrite(timer2, 700000, true);
  timerAlarmWrite(timer3, 300000, true);

  timerAlarmEnable(timer0);
  timerAlarmEnable(timer1);
  timerAlarmEnable(timer2);
  timerAlarmEnable(timer3);
}
void loop() {
  if (interruptCounter_0 > 0) {
    portENTER_CRITICAL(&timerMux0);
    interruptCounter_0--;
    portEXIT_CRITICAL(&timerMux0);
    
    perdNum++;
    Serial.print("Periode ");
    Serial.print(perdNum);
    Serial.println(":");
    Serial.println("Led 1 "+String(digitalRead(PIN1)));
    Serial.println("Led 2 "+String(digitalRead(PIN2)));
    Serial.println("Led 3 "+String(digitalRead(PIN3)));
  }
}