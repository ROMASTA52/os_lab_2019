#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Функция для вычисления произведения по модулю
uint64_t MultModulo(uint64_t a, uint64_t b, uint64_t mod);

// Функция для преобразования строки в uint64_t
bool ConvertStringToUI64(const char *str, uint64_t *val);

#endif // UTILS_H
