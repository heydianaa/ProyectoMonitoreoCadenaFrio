// Universidad del Valle de Guatemala
// Electronica Digital 2
// Diana Mendez 241341
// Proyecto 1: Sistema de monitoreo para cadena de frío

#include <Arduino.h>
#include "driver/gpio.h"
#include <stdint.h>
#include "driver/ledc.h"

#define boton GPIO_NUM_35
#define sensor GPIO_NUM_34

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
