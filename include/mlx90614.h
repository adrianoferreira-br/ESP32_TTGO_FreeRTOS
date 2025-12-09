#ifndef MLX90614_H
#define MLX90614_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MLX90614.h>

// Definição dos pinos I2C para o MLX90614
#define MLX90614_SDA 32
#define MLX90614_SCL 33

// Variáveis globais para armazenar as temperaturas
extern float temperatura_ambiente_mlx;
extern float temperatura_objeto_mlx;

// Funções do sensor MLX90614
void mlx90614_setup();
void mlx90614_loop();
float mlx90614_read_ambient();
float mlx90614_read_object();

#endif // MLX90614_H
