#include <Arduino.h>

//GPIO 35
#define VOLTAJE_PIN 32
#define CORRIENTE_PIN 33

int freqMedida = 0;

int voltajeMedido = 0;
int corrienteMedida = 0;
int voltajemV = 0;
int corrientemA = 0;
int potenciauW = 0;
int consumo = 0;

int voltajeMax = 17600;
int corrienteMax = 5588;


void setup() {
  Serial.begin(9600);
  pinMode(VOLTAJE_PIN, INPUT);
  pinMode(CORRIENTE_PIN, INPUT);
}

void loop(){
    voltajeMedido = analogRead(VOLTAJE_PIN);
  corrienteMedida = analogRead(CORRIENTE_PIN);

  voltajemV = map(voltajeMedido, 0, 4095, 0, voltajeMax);
  corrientemA = map(corrienteMedida, 0, 4095, 0, corrienteMax);

  //Serial.print(voltajemV + " mV | ");
  //Serial.print(corrientemA + "mA | ");
  Serial.print(voltajemV);
  Serial.print("mV | ");
  Serial.print(corrientemA);
  Serial.print("mA | ");
  potenciauW = voltajemV*corrientemA;
  //Serial.println(potenciauW/1000 + "mW");
  Serial.print(potenciauW/1000);
  Serial.println("mW");
  delay(500);
}
