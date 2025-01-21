#pragma once

#include "Adafruit_ST7789.h"

void cleanDisplay(Adafruit_ST7789& display);
void drawThermometerIcon(Adafruit_ST7789& display, int color, int x, int y);
void drawDropletIcon(Adafruit_ST7789& display, int color, int x, int y);
void drawMovementIcon(Adafruit_ST7789& display, int color, int x, int y);
void drawRotationIcon(Adafruit_ST7789& display, int color, int x, int y);
void drawPressureIcon(Adafruit_ST7789& display, int color, int x, int y);
void drawMessage(Adafruit_ST7789& display, const GFXfont *font, int x, int y, int color, String message);
