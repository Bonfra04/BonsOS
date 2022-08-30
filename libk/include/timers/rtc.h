#pragma once

#include <stdint.h>

void rtc_init();

uint8_t rtc_seconds();
uint8_t rtc_minutes();
uint8_t rtc_hours();
uint8_t rtc_weekday();
uint8_t rtc_monthday();
uint8_t rtc_month();
uint16_t rtc_year();
