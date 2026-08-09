// Universidad del Valle de Guatemala
// Electronica Digital 2
// Diana Mendez 241341
// Proyecto 1: Sistema de monitoreo para cadena de frío

#include <Arduino.h>
#include "driver/gpio.h"
#include <stdint.h>
#include "driver/ledc.h"

#define boton GPIO_NUM_34
#define sensor 36

#define ledroja  GPIO_NUM_32
#define ledverde GPIO_NUM_33
#define ledazul  GPIO_NUM_25

#define servomotor GPIO_NUM_13

#define a GPIO_NUM_16
#define b GPIO_NUM_17
#define c GPIO_NUM_18
#define d GPIO_NUM_19
#define e GPIO_NUM_21
#define f GPIO_NUM_22
#define g GPIO_NUM_23

#define dispd GPIO_NUM_26
#define dispu GPIO_NUM_27
#define dispdec GPIO_NUM_14

// ADC
#define ADC_VREF_mV 3300.0
#define ADC_RESOLUTION 4096.0

// Banderas
volatile bool leerTemperatura = false;
volatile uint32_t tiempoBoton = 0;
const uint32_t tiempoAntirrebote = 50;
portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

// ----Variables----

float temperatura = 0;


// ----Funciones----
void mostrarTemperatura(float temperatura);
void IRAM_ATTR botonISR();
float leerTemp();
void setup() {

  Serial.begin(115200);
  gpio_set_direction(boton, GPIO_MODE_INPUT);
  gpio_set_pull_mode(boton, GPIO_FLOATING);
  pinMode(sensor, INPUT);

  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);

  // Interrupcion del boton
  attachInterrupt(
    digitalPinToInterrupt((uint8_t)boton),
    botonISR,
    CHANGE
  );

  gpio_set_direction(ledroja, GPIO_MODE_OUTPUT);
  gpio_set_direction(ledverde, GPIO_MODE_OUTPUT);
  gpio_set_direction(ledazul, GPIO_MODE_OUTPUT);

  gpio_set_direction(servomotor, GPIO_MODE_OUTPUT);

  gpio_set_direction(a, GPIO_MODE_OUTPUT);
  gpio_set_direction(b, GPIO_MODE_OUTPUT);
  gpio_set_direction(c, GPIO_MODE_OUTPUT);
  gpio_set_direction(d, GPIO_MODE_OUTPUT);
  gpio_set_direction(e, GPIO_MODE_OUTPUT);
  gpio_set_direction(f, GPIO_MODE_OUTPUT);
  gpio_set_direction(g, GPIO_MODE_OUTPUT);

  gpio_set_level(ledroja, 0);
  gpio_set_level(ledverde, 0);
  gpio_set_level(ledazul, 0);
  mostrarTemperatura(temperatura);
}

void loop() {

  bool medir = false;
  // Leer bandera
  portENTER_CRITICAL(&mux);
  medir = leerTemperatura;
  // Limpiar bandera
  leerTemperatura = false;
  portEXIT_CRITICAL(&mux);
  
  // Si se presiono el boton
  if (medir) {

    float temperatura = leerTemp();
    Serial.print(temperatura);
  }
}

// Interrupcion del boton
void IRAM_ATTR botonISR() {
  uint32_t tiempoActual = millis();
  uint32_t diferencia = tiempoActual - tiempoBoton;
  tiempoBoton = tiempoActual;

  if (
    diferencia >= tiempoAntirrebote &&
    gpio_get_level(boton) == 1
  ) {
    portENTER_CRITICAL_ISR(&mux);
    leerTemperatura = true;
    portEXIT_CRITICAL_ISR(&mux);
  }
}

//Sensor 
float leerTemp() {

  // ADC
  int adcVal = analogRead(sensor);

  // ADC a mV
  float milliVolt =
    adcVal * (ADC_VREF_mV / ADC_RESOLUTION);

  // LM35: 10 mV = 1 °C
  float tempC = milliVolt / 10.0;

  return tempC;
}
