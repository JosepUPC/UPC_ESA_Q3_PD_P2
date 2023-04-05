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