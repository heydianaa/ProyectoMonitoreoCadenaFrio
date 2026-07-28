// Universidad del Valle de Guatemala
// Electronica Digital 2
// Diana Mendez 241341
// Proyecto 1: Sistema de monitoreo para cadena de frío

#include <Arduino.h>
#include "driver/gpio.h"
#include <stdint.h>

#define boton GPIO_NUM_33
#define sensor GPIO_NUM_33

#define ledroja  GPIO_NUM_33
#define ledverde GPIO_NUM_33
#define ledazul  GPIO_NUM_33

#define servomotor GPIO_NUM_33

#define a GPIO_NUM_33
#define b GPIO_NUM_33
#define c GPIO_NUM_33
#define d GPIO_NUM_33
#define e GPIO_NUM_33
#define f GPIO_NUM_33
#define g GPIO_NUM_33

// ----Variables----

float temperatura = 0;


// ----Funciones----
void mostrarTemperatura(float temperatura);

void setup() {

  gpio_set_direction(boton, GPIO_MODE_INPUT);
  gpio_set_direction(sensor, GPIO_MODE_INPUT);

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
