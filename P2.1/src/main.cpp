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