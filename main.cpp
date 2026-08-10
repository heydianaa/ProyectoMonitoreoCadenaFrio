// Universidad del Valle de Guatemala
// Electronica Digital 2
// Diana Mendez 241341
// Proyecto 1: Sistema de monitoreo para cadena de frío

#include <Arduino.h>
#include "driver/gpio.h"
#include "driver/ledc.h"
#include <stdint.h>
#include "config.h"

// Pines
#define boton GPIO_NUM_34
#define sensor 36

#define ledroja  GPIO_NUM_32
#define ledverde GPIO_NUM_33
#define ledazul  GPIO_NUM_25

#define a GPIO_NUM_22
#define b GPIO_NUM_23
#define c GPIO_NUM_4
#define d GPIO_NUM_17
#define e GPIO_NUM_18
#define f GPIO_NUM_21
#define g GPIO_NUM_19

#define DISPLAY_1 GPIO_NUM_26
#define DISPLAY_2 GPIO_NUM_27
#define DISPLAY_3 14

// ADC
#define ADC_VREF_mV 3300.0
#define ADC_RESOLUTION 4096.0

// PWM LED RGB
#define canalRojo  LEDC_CHANNEL_0
#define canalVerde LEDC_CHANNEL_1
#define canalAzul  LEDC_CHANNEL_2
#define timerRGB LEDC_TIMER_0

// Adafruit IO
AdafruitIO_Feed *canalTemperatura = io.feed("temperatura");

// Banderas
volatile bool leerTemperatura = false;
volatile uint32_t tiempoBoton = 0;
const uint32_t tiempoAntirrebote = 50;
portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

// Variables displays
int decenas = 0;
int unidades = 0;
int decimal = 0;
uint8_t digitoActual = 0;
unsigned long tiempoMux = 0;

// Segmentos
const uint8_t numeros[10][7] = {
  {1,1,1,1,1,1,0}, // 0
  {0,1,1,0,0,0,0}, // 1
  {1,1,0,1,1,0,1}, // 2
  {1,1,1,1,0,0,1}, // 3
  {0,1,1,0,0,1,1}, // 4
  {1,0,1,1,0,1,1}, // 5
  {1,0,1,1,1,1,1}, // 6
  {1,1,1,0,0,0,0}, // 7
  {1,1,1,1,1,1,1}, // 8
  {1,1,1,1,0,1,1}  // 9
};

// Funciones
void IRAM_ATTR botonISR();
float leerTemp();
void revisarTemperatura(float temperatura);

// LED RGB
void configurarRGB();
void configurarCanal(gpio_num_t pin, ledc_channel_t canal, ledc_timer_t timer);
void escribirRGB(uint8_t rojo, uint8_t verde, uint8_t azul);
void apagarLED();
void encenderRojo();
void encenderVerde();
void encenderAzul();
void encenderGelb();

// Displays
void separarTemperatura(float temperatura);
void mostrarNumero(uint8_t numero);
void apagarDisplays();
void apagarSegmentos();
void multiplexarDisplays();


// ========================
// SETUP
// ========================

void setup() {

  Serial.begin(115200);

  // Boton
  gpio_set_direction(boton, GPIO_MODE_INPUT);
  gpio_set_pull_mode(boton, GPIO_FLOATING);

  // Sensor LM35
  pinMode(sensor, INPUT);
  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);

  // LED RGB
  configurarRGB();
  apagarLED();

  // Segmentos
  gpio_set_direction(a, GPIO_MODE_OUTPUT);
  gpio_set_direction(b, GPIO_MODE_OUTPUT);
  gpio_set_direction(c, GPIO_MODE_OUTPUT);
  gpio_set_direction(d, GPIO_MODE_OUTPUT);
  gpio_set_direction(e, GPIO_MODE_OUTPUT);
  gpio_set_direction(f, GPIO_MODE_OUTPUT);
  gpio_set_direction(g, GPIO_MODE_OUTPUT);

  // Control displays
  gpio_set_direction(DISPLAY_1, GPIO_MODE_OUTPUT);
  gpio_set_direction(DISPLAY_2, GPIO_MODE_OUTPUT);
  pinMode(DISPLAY_3, OUTPUT);

  apagarDisplays();
  apagarSegmentos();
  separarTemperatura(0.0);

  // Interrupcion
  attachInterrupt(digitalPinToInterrupt((uint8_t)boton), botonISR, CHANGE);

  // Adafruit IO
  Serial.print("Conectando con Adafruit IO");
  io.connect();

  while (io.status() < AIO_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println();
  Serial.println(io.statusText());
  Serial.println("Sistema listo");
}

void loop() {

  multiplexarDisplays();
  io.run();

  bool medir = false;

  portENTER_CRITICAL(&mux);
  medir = leerTemperatura;
  leerTemperatura = false;
  portEXIT_CRITICAL(&mux);

  if (medir) {

    float temperatura = leerTemp();

    separarTemperatura(temperatura);
    revisarTemperatura(temperatura);

    Serial.print("Temperatura: ");
    Serial.print(temperatura);
    Serial.println(" C");

    Serial.print("Displays: ");
    Serial.print(decenas);
    Serial.print(unidades);
    Serial.println(decimal);

    Serial.print("Enviando a Adafruit IO: ");
    Serial.println(temperatura);

    canalTemperatura->save(temperatura);
  }
}

void IRAM_ATTR botonISR() {

  uint32_t tiempoActual = millis();
  uint32_t diferencia = tiempoActual - tiempoBoton;

  tiempoBoton = tiempoActual;

  if (diferencia >= tiempoAntirrebote && gpio_get_level(boton) == 1) {

    portENTER_CRITICAL_ISR(&mux);
    leerTemperatura = true;
    portEXIT_CRITICAL_ISR(&mux);
  }
}

float leerTemp() {

  int adcVal = analogRead(sensor);
  float milliVolt = adcVal * (ADC_VREF_mV / ADC_RESOLUTION);
  float tempC = milliVolt / 10.0;

  return tempC;
}

void revisarTemperatura(float temperatura) {

  if (temperatura < 23.0) {
    encenderAzul();
  }

  else if (temperatura < 25.0) {
    encenderVerde();
  }

  else if (temperatura < 27.0) {
    encenderGelb();
  }

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

  configurarCanal(ledroja, canalRojo, timerRGB);
  configurarCanal(ledverde, canalVerde, timerRGB);
  configurarCanal(ledazul, canalAzul, timerRGB);
}

void configurarCanal(gpio_num_t pin, ledc_channel_t canal, ledc_timer_t timer) {

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

void escribirRGB(uint8_t rojo, uint8_t verde, uint8_t azul) {

  ledc_set_duty(LEDC_HIGH_SPEED_MODE, canalRojo, rojo);
  ledc_update_duty(LEDC_HIGH_SPEED_MODE, canalRojo);

  ledc_set_duty(LEDC_HIGH_SPEED_MODE, canalVerde, verde);
  ledc_update_duty(LEDC_HIGH_SPEED_MODE, canalVerde);

  ledc_set_duty(LEDC_HIGH_SPEED_MODE, canalAzul, azul);
  ledc_update_duty(LEDC_HIGH_SPEED_MODE, canalAzul);
}

void apagarLED() {
  escribirRGB(0, 0, 0);
}

void encenderRojo() {
  escribirRGB(255, 0, 0);
}

void encenderVerde() {
  escribirRGB(0, 255, 0);
}

void encenderAzul() {
  escribirRGB(0, 0, 255);
}

void encenderGelb() {
  escribirRGB(255, 170, 0);
}

void separarTemperatura(float temperatura) {

  int temp = (int)(temperatura * 10.0 + 0.5);

  if (temp < 0) {
    temp = 0;
  }

  if (temp > 999) {
    temp = 999;
  }

  decenas = temp / 100;
  unidades = (temp / 10) % 10;
  decimal = temp % 10;
}

void mostrarNumero(uint8_t numero) {

  gpio_set_level(a, numeros[numero][0]);
  gpio_set_level(b, numeros[numero][1]);
  gpio_set_level(c, numeros[numero][2]);
  gpio_set_level(d, numeros[numero][3]);
  gpio_set_level(e, numeros[numero][4]);
  gpio_set_level(f, numeros[numero][5]);
  gpio_set_level(g, numeros[numero][6]);
}

void apagarSegmentos() {

  gpio_set_level(a, LOW);
  gpio_set_level(b, LOW);
  gpio_set_level(c, LOW);
  gpio_set_level(d, LOW);
  gpio_set_level(e, LOW);
  gpio_set_level(f, LOW);
  gpio_set_level(g, LOW);
}

void apagarDisplays() {

  gpio_set_level(DISPLAY_1, LOW);
  gpio_set_level(DISPLAY_2, LOW);
  digitalWrite(DISPLAY_3, LOW);
}

// MULTIPLEAXADO
void multiplexarDisplays() {

  if (micros() - tiempoMux < 500) {
    return;
  }

  tiempoMux = micros();
  apagarDisplays();
  apagarSegmentos();
  delayMicroseconds(10);

  switch (digitoActual) {

    case 0:
      mostrarNumero(decenas);
      delayMicroseconds(5);
      gpio_set_level(DISPLAY_1, HIGH);
      break;

    case 1:
      mostrarNumero(unidades);
      delayMicroseconds(5);
      gpio_set_level(DISPLAY_2, HIGH);
      break;

    case 2:
      mostrarNumero(decimal);
      delayMicroseconds(5);
      digitalWrite(DISPLAY_3, HIGH);
      break;
  }
  digitoActual++;
  if (digitoActual > 2) {
    digitoActual = 0;
  }
}
