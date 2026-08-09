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

// PWM LED RGB
#define canalRojo LEDC_CHANNEL_0
#define canalVerde LEDC_CHANNEL_1
#define canalAzul LEDC_CHANNEL_2

#define timerRGB LEDC_TIMER_0

// Banderas
volatile bool leerTemperatura = false;
volatile uint32_t tiempoBoton = 0;
const uint32_t tiempoAntirrebote = 50;
portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

// ----Variables----

float temperatura = 0;


// ----Funciones----
void IRAM_ATTR botonISR();
float leerTemp();
void revisarTemperatura(float temperatura);
void configurarRGB();
void configurarCanal(
  gpio_num_t pin,
  ledc_channel_t canal,
  ledc_timer_t timer
);

void escribirRGB(
  uint8_t rojo,
  uint8_t verde,
  uint8_t azul
);

void apagarLED();
void encenderRojo();
void encenderVerde();
void encenderAzul();
void encenderGelb();

void setup() {

  Serial.begin(115200);
  gpio_set_direction(boton, GPIO_MODE_INPUT);
  gpio_set_pull_mode(boton, GPIO_FLOATING);
  pinMode(sensor, INPUT);

  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);

  configurarRGB();
  apagarLED();

  // Interrupcion del boton
  attachInterrupt(
    digitalPinToInterrupt((uint8_t)boton),
    botonISR,
    CHANGE
  );

  gpio_set_direction(servomotor, GPIO_MODE_OUTPUT);

  gpio_set_direction(a, GPIO_MODE_OUTPUT);
  gpio_set_direction(b, GPIO_MODE_OUTPUT);
  gpio_set_direction(c, GPIO_MODE_OUTPUT);
  gpio_set_direction(d, GPIO_MODE_OUTPUT);
  gpio_set_direction(e, GPIO_MODE_OUTPUT);
  gpio_set_direction(f, GPIO_MODE_OUTPUT);
  gpio_set_direction(g, GPIO_MODE_OUTPUT);
  
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
    revisarTemperatura(temperatura);
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

float leerTemp() {  //Sensor 
  // ADC
  int adcVal = analogRead(sensor);
  // ADC a mV
  float milliVolt =
    adcVal * (ADC_VREF_mV / ADC_RESOLUTION);
  // LM35: 10 mV = 1 °C
  float tempC = milliVolt / 10.0;
  return tempC;
}

//Revisar Temperatura
void revisarTemperatura(float temperatura) {
  // Menor de 23 °C
  if (temperatura < 23.0) {
    encenderAzul();
  }
  // 23 °C - 25 °C
  else if (temperatura < 25.0) {
    encenderVerde();
  }
  // 25 °C - 27 °C
  else if (temperatura < 27.0) {
    encenderGelb();
  }
  // 27 °C o mas
  else {
    encenderRojo();
  }
}

void configurarRGB() { 
  ledc_timer_config_t configTimer = {};
  configTimer.speed_mode = LEDC_HIGH_SPEED_MODE;
  configTimer.duty_resolution = LEDC_TIMER_8_BIT;
  configTimer.timer_num = timerRGB;
  configTimer.freq_hz = 5000;
  configTimer.clk_cfg = LEDC_AUTO_CLK;
  ledc_timer_config(&configTimer);
  configurarCanal(
    ledroja,
    canalRojo,
    timerRGB
  );
  configurarCanal(
    ledverde,
    canalVerde,
    timerRGB
  );

  configurarCanal(
    ledazul,
    canalAzul,
    timerRGB
  );
}

void configurarCanal(
  gpio_num_t pin,
  ledc_channel_t canal,
  ledc_timer_t timer
) {

  ledc_channel_config_t configCanal = {};
  configCanal.gpio_num = pin;
  configCanal.speed_mode = LEDC_HIGH_SPEED_MODE;
  configCanal.channel = canal;
  configCanal.intr_type = LEDC_INTR_DISABLE;
  configCanal.timer_sel = timer;
  configCanal.duty = 0;
  configCanal.hpoint = 0;
  ledc_channel_config(&configCanal);
}

void escribirRGB(
  uint8_t rojo,
  uint8_t verde,
  uint8_t azul
) {
  ledc_set_duty(LEDC_HIGH_SPEED_MODE, canalRojo, rojo);
  ledc_update_duty(LEDC_HIGH_SPEED_MODE, canalRojo);
  ledc_set_duty(LEDC_HIGH_SPEED_MODE, canalVerde, verde);
  ledc_update_duty(LEDC_HIGH_SPEED_MODE, canalVerde);
  ledc_set_duty(LEDC_HIGH_SPEED_MODE, canalAzul, azul);
  ledc_update_duty(LEDC_HIGH_SPEED_MODE, canalAzul);
}
void apagarLED() {  //LED APAGADA
  escribirRGB(0, 0, 0);
}
void encenderRojo() { //LED EN ROJO
  escribirRGB(255, 0, 0);
}
void encenderVerde() { //LED EN VERDE
  escribirRGB(0, 255, 0);
}
void encenderAzul() { //LED EN AZUL
  escribirRGB(0, 0, 255);
}
void encenderGelb() { //LED EN AMARILLO
  escribirRGB(255, 100, 0);
}
