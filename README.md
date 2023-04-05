# Practica 1

## Indice

* Objetivo general
* Tasks
    - Interrupcion por GPIO
    - Interrupción por Timer
    - LED Interrupt
* Diagrama de flujo
* Diagrama de tiempos
* Conclusiones
___

## **OBjetivo general**

Bienvenidos a la 2n Pràctica del curso de procesadores **Processadores Digitales**. En esta session vamos a ver una mecànica fundamental de los microprocesadores, las Interrupciones. És decir, funciones que se ejectuan prioritariamente si se cumplen una sèrie de especificaciones. Vamos anarlizar un código para varios casos.

---

-   *Si hay alguna duda de como ejecutar el platformio o trabajar con GitHub, te recomiendo mirar el README-md de la 1a pràctica*

## **Tasks**

### **Interrupcion por GPIO**

```C
//PROCESADO DIGITAL
//PRÀCTICA 2.1
//Iniciamos las librerias de Arduino para la pràctica
#include <Arduino.h>

// Creamos una clase primitiva que contega el numero del pin (connectado a un interruptor) que queremos interrumpir, un contador que monitorize el numero de veces que lo activamos y por ultimo una booleana que exprese el estado del Pin.

struct Button {
  const uint8_t PIN;
  uint32_t numberKeyPresses;
  bool pressed;
};

// Declaramos incialmente las variables del proyecto. En este caso una variable con los valores por default de la clase creada anteiormente y una nueva variable que controle el tiempo de ejecucion del programa. És muy importante inicializar la boolean en falso y no equivocarse con el pin selecionado. 
Button button1 = {18, 0, false};
static uint32_t lastMillis = 0;

// Antes de la setup delaramos la funcion que queremos que se ejecute luego de interrupcion. Para este caso queremos acumular un valor en el contador y cambiar el estado del boolean en verdadero. Muy importante tener encuenta que las interrupciones se ejecutan por encima de todo, posponiendo qualquier accion ejecutado en la setup o el loop.
void IRAM_ATTR isr() {
  button1.numberKeyPresses += 1;
  button1.pressed = true;
}

// Finalmente, empezamos inicializando el Serial para la conexion con el dispostivo Esp32, declaramos el pin de la clase creada anteiormente que detecte qualquier señal se subido del pin y la funcion AttachInterrup(), que llamara a ejecutar la funcion de interrupcion si es que se detecta un flanco de bajada en la conexion con el pin. 
void setup() {
  Serial.begin(115200);
  pinMode(button1.PIN, INPUT_PULLUP);
  attachInterrupt(button1.PIN, isr, FALLING);
}

// En el loop monitorizaremos los eventos que pasen
// Por un lado queremos saber quando se ha activado el interruptor. Tenemos una condicion que solo se cumplirar si és que el booleando de la variable és verdadero. En caso de que lo sea, se enviara una mensaje conforme se a activado el interruptor tantas veces como las acumuladas en el contador. Como hemos incializado los valores de la clase en falso, no se cumple la condicion. Unicamente se cumple si se recibe un pulso que active la funcion de interrupcion que a su vez modifique el estado del booleano de la clase. És decir, primero hemos de activar las interrupciones que modifiquen ciertos valores para luego poder activar ciertas operaciones inhabilitadas o cambiar ciertos resultados
void loop() {
  if (button1.pressed) {
    Serial.printf("Button 1 has been pressed %u times\n", button1.numberKeyPresses);
    button1.pressed = false;
  }
  // Por otro lado, necessitamos una funcion que inhabilite el interrupt. Para ese tenemos el DetachInterrupt, que solo se activara quando haya pasado 1 minuto despues de haber arrancado el programa.

  if (millis() - lastMillis > 60000) {
    lastMillis = millis();
    detachInterrupt(button1.PIN);
    Serial.println("Interrupt Detached!");
  }
}
```

### **Interrupción por Timer**

```C
//PROCESADO DIGITAL
//PRÀCTICA 2.2

//Inicializamos la libreria e Arduino
#include <Arduino.h>

//Declaramos una variable int para contar los estados de interrupcion del timer del Esp32 y un int total para contar el total de interrupciones del timer. El primero se declara volátil pues variar rapidamente entre valores similares constantemente.
volatile int interruptCounter;
int totalInterruptCounter;

/// Declaramos por default la variable *timer* que controlara uno de los timers de Esp32. Tambien declaramos una variabel timerMUX para controlar su configuracion
hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

/// Escribimos la accion del timerInterrupt. En este caso queremos acumular un 1 al estado de interrupcion del timer. Cabe destacar que ultilizamos "portENTER_CRITICAL_ISR" y "portEXIT_CRITICAL_ISR" para controlar de forma critica el cambio de valor de la variable, pues esta cambia tanto en las interrupciones como en el main loop.
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

/// Seguidamente declaramos la funcion timerAttachInterrup para arrancar la accion timerInterrupt y en que momento queremos que se interrupa el proceso. En este caso en la subida del contador
  timerAttachInterrupt(timer, &onTimer, true);

/// Con la función timerAlarmWrite declaramos la configuracion de la interrupcion. Es decir,  cada quanto queremos que se interrupa el proceso loop i si lo queremos hacer de forma periódica. Finalmente timerAlarmEnable() abilita la Interrupcion
  timerAlarmWrite(timer, 500000, true);
  timerAlarmEnable(timer);
}

void loop() {

/// Dentro del main loop, declaramos una condicion que solo se cumplirar en el caso de que la variable que suma las interrupciones acumule 1 interrupcion.
  if (interruptCounter > 0) {

/// Si sucede, dentro de l'estado critico del timer, variamos el valor para reducir-lo a zero y activar la condicion unicamente quando se detecte una interrupcion
    portENTER_CRITICAL(&timerMux);
    interruptCounter--;
    portEXIT_CRITICAL(&timerMux);

/// Por otro lado, sumamos una interrupcion a la variable total de interrupciones y mostramos por pantalla el total que por definicion se verá incrementado
    totalInterruptCounter++;
    Serial.print("An interrupt as occurred. Total number: ");
    Serial.println(totalInterruptCounter);
  }
}
```

### **LED Interrupt**

```C
//PROCESADO DIGITAL
//PRÀCTICA 2.3

//Iniciamos las librerias de Arduino para la pràctica
#include <Arduino.h>

//Delcaramos una variable inta para controlar con la funcion timer el tiempo de aparicion por pantalla del estado de los leds y uno para controlar el numero de perdïodos que llebamos
volatile int interruptCounter_0;
int perdNum;

// Declaramos las variables que contiene el numero de los pines que usaremos, la variable timer por default que controlara el ciclo de cambio de estado de los leds y cada una de sus configuariones timerMUX.
const uint8_t PIN1=18, PIN2=19, PIN3=21;
hw_timer_t * timer0 = NULL;
hw_timer_t * timer1 = NULL;
hw_timer_t * timer2 = NULL; 
hw_timer_t * timer3 = NULL;
portMUX_TYPE timerMux0 = portMUX_INITIALIZER_UNLOCKED;
portMUX_TYPE timerMux1 = portMUX_INITIALIZER_UNLOCKED;
portMUX_TYPE timerMux2 = portMUX_INITIALIZER_UNLOCKED;
portMUX_TYPE timerMux3 = portMUX_INITIALIZER_UNLOCKED;

//Ahora declaramos las acciones que ejecutaremos a la hora de las interrupciones. Lo que queremos hacer es cambiar el estado de cada uno de los les assigando a exception del primero (interruptCounter_0), que alerara la variable de estado de la salida por pantalla.
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

// Iniciamos el Serial y declaramos la naturaleza de nuestros pines. Por otro lado, iniciamos cada uno de los timer del Esp32 del 0-3 i los enlaçamos a la accion declarada anteriormente con el timerattachInterrupt
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

// Con el timerAlamrWrite indicaremos la frequencia con la queremos que el proceso sea interrumpido y finalmente abilitaremos el timer. 
  timerAlarmWrite(timer0, 500000, true);
  timerAlarmWrite(timer1, 1100000, true);
  timerAlarmWrite(timer2, 700000, true);
  timerAlarmWrite(timer3, 300000, true);

  timerAlarmEnable(timer0);
  timerAlarmEnable(timer1);
  timerAlarmEnable(timer2);
  timerAlarmEnable(timer3);
}

// Como los interrupts controlan el estado de cada uno de los leds de forma periòdica, en el loop main unicamente mostraremos por pantalla es lestado de los leds utilizando el periodo del primer timer
void loop() {
  if (interruptCounter_0 > 0) {
    portENTER_CRITICAL(&timerMux0);
    interruptCounter_0--;
    portEXIT_CRITICAL(&timerMux0);
    
    perdNum++;
    Serial.print("Periode ");
    Serial.print(perdNum);
    Serial.println(":");
    Serial.print("Led 1 ");
    Serial.println(digitalRead(PIN1));
    Serial.print("Led 2 ");
    Serial.println(digitalRead(PIN2));
    Serial.print("Led 3 ");
    Serial.println(digitalRead(PIN3));
  }
}
```

## **Diagrama de flujo**

## **Diagrama de tiempos**

## **Conclusiones**

>   *--- Las interrupciones son herramientas muy utiles ha a hora de querer generar respuestas de forma rápida y muy importante: prioritaria. *

> *--- Se pueden dar en casos puntuales, que respondan a la llamade de un cierto impulso, ya sea una llamade emergencia, una alarma, un sensor de temperatura indicando el maximo de resistencia de un sistema junto a sus medidas. Las medidas pueden ir desde enviar un mensaje, modificar deritas vairables o activar ciertas funciones o prodecimentos*

> *--- Por otro grácias a la herramienta timer del Esp33 se pueden generar interrupciones de los procesos main loop de forma periódica. Muy útil a la hora de querer ejecutar ciertas acciones en un tiempo constante hasta un maximo de 4*. Las aplicaciones pueden ir desde recodificar valores cada cierto tiempo, comprobar el estado del sistema, crear un historial,...etc. 