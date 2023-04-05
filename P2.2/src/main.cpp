//PROCESADO DIGITAL
//PRÀCTICA 2.2

//Inicializamos la libreria e Arduino
#include <Arduino.h>

//Declaramos una variable int para contar los estados de interrupcion del timer del Esp32 i un int total para contar el total de interrupciones del timer. El primero se declara volatil pues variara rapidamente entre valores similares constantemente
volatile int interruptCounter;
int totalInterruptCounter;

/// Declaramos por default la variable *timer* que controlara uno de los timers de Esp32. Tambien declaramos una variabel timerMUX para controlar la configuracion del timer.
hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

/// Escribimos la accion del timerInterrupt. En este caso queremos acumular un 1 al estado de interrupcion del timer. Cave desdtacar que ultilizamos "portENTER_CRITICAL_ISR" y "portEXIT_CRITICAL_ISR" para controlar de forma critica el cambio de valor de la variable, pues esta cambia tanto en las interrupciones como en el main loop.
void IRAM_ATTR onTimer() {
  portENTER_CRITICAL_ISR(&timerMux);
  interruptCounter++;
  portEXIT_CRITICAL_ISR(&timerMux);
}


void setup() {

/// inicializamos el serial de arduino para conectarse con el Esp32
  Serial.begin(115200);

/// inicialitzamos el timerBegin con la variable declarada anteriormente. El primer valor representa el numero de timer que queremos ocupara. Esp32 tiene 4. El segundo valor controla la frequencia a la queremos que cuente. En este caso lo queremos a 80= 80MHz. El ultimo valor "true" indica el flanco por el que queremos contar, siendo "true" de subida i "false" de bajada
  timer = timerBegin(0, 80, true);

/// Seguidamente declaramos la funcion timerAttachInterrup para arrancar la accion timerInterrut y en que momento queremos que se interrupa el proceso. En este caso en la subida del contador
  timerAttachInterrupt(timer, &onTimer, true);

/// Con la función timerAlarmWrite declaramos la configuracion de la interrupcion. Es decir,  cada quanto queremos que se interrupa el proceso loop i si lo queremos hacer de forma periódica. Finalmente timerAlarmEnable() abilita la Interrupcion
  timerAlarmWrite(timer, 500000, true);
  timerAlarmEnable(timer);
}

void loop() {

/// Dentro del main loop, declaramos una condicion que solo se cumplirar en el caso de que la variable que suma las interrupciones acumule 1 interrupcion.
  if (interruptCounter > 0) {

/// Si sucede, dentro de l'estado critico del timer, variamos el valor para reducir-lo a zero para activar la condicion unicamente quando se detecte una interrupcion
    portENTER_CRITICAL(&timerMux);
    interruptCounter--;
    portEXIT_CRITICAL(&timerMux);

/// Por otro lado, con sumamos una interrupcion a la variable total de interrupciones y mostramos por pantalla el total que por definicion se verá incrementado
    totalInterruptCounter++;
    Serial.print("An interrupt as occurred. Total number: ");
    Serial.println(totalInterruptCounter);
  }
}