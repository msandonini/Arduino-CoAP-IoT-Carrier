#include "CarrierGfxDrawFunctions.h"


void cleanDisplay(Adafruit_ST7789& display) {
    display.fillScreen(0x0000);
}

void drawThermometerIcon(Adafruit_ST7789& display, int color, int x, int y) {
    display.fillCircle(x+15, y+10, 10, color);
    display.fillRect(x+5, y+10, 21, 28, color);
    display.fillCircle(x+15, y+45, 15, color);

    display.fillCircle(x+15, y+10, 7, 0x0000);
    display.fillRect(x+8, y+13, 15, 25, 0x0000);
    display.fillCircle(x+15, y+45, 12, 0x0000);
}

void drawDropletIcon(Adafruit_ST7789& display, int color, int x, int y) {
    display.fillTriangle(x+2, y+30, x+20, y, x+38, y+30, color);
    display.fillCircle(x+20, y+40, 20, color);
    display.fillTriangle(x+6, y+30, x+20, y+5, x+34, y+30, 0x0000);
    display.fillCircle(x+20, y+40, 17, 0x0000);
}

void drawMovementIcon(Adafruit_ST7789& display, int color, int x, int y) {
    display.fillTriangle(x+20, y+15, x+30, y, x+40, y+15, color);      // North
    display.fillTriangle(x+25, y+12, x+30, y+5, x+35, y+12, 0x0000);
    display.fillTriangle(x, y+30, x+15, y+20, x+15, y+40, color);      // West
    display.fillTriangle(x+5, y+30, x+12, y+25, x+12, y+35, 0x0000);
    display.fillTriangle(x+20, y+45, x+40, y+45, x+30, y+60, color);   // South
    display.fillTriangle(x+25, y+48, x+35, y+48, x+30, y+55, 0x0000);
    display.fillTriangle(x+45, y+40, x+45, y+20, x+60, y+30, color);   // East
    display.fillTriangle(x+48, y+35, x+48, y+25, x+55, y+30, 0x0000);
    display.fillRect(x+15, y+25, 30, 10, color);                         // Horizontal
    display.fillRect(x+25, y+15, 10, 30, color);                         // Vertical
    display.fillRect(x+12, y+28, 36, 4, 0x0000);
    display.fillRect(x+28, y+12, 4, 36, 0x0000);
}

void drawRotationIcon(Adafruit_ST7789& display, int color, int x, int y) {
    display.fillCircle(x+30, y+30, 27, color);
    display.fillCircle(x+30, y+30, 24, 0x0000);
    display.fillCircle(x+30, y+30, 20, color);
    display.fillCircle(x+30, y+30, 17, 0x0000);

    display.fillTriangle(x, y, x+30, y, x+30, y+30, 0x0000);

    display.fillTriangle(x+15, y+7, x+30, y, x+30, y+15, color);
    display.fillTriangle(x+22, y+7, x+27, y+5, x+27, y+10, 0x0000);

    display.fillRect(x+27, y+6, 4, 4, 0x0000);

    display.fillTriangle(x+12, y+12, x+17, y+17, x+12, y+15, color);

    display.fillCircle(x+30, y+30, 5, color);
    display.fillCircle(x+30, y+30, 2, 0x0000);
}

void drawPressureIcon(Adafruit_ST7789& display, int color, int x, int y) {
    display.fillRect(x, y+50, 60, 10, color);

    display.fillTriangle(x+5, y+35, x+25, y+35, x+15, y+50, color);
    display.fillTriangle(x+35, y+35, x+55, y+35, x+45, y+50, color);

    display.fillRect(x+10, y, 10, 35, color);
    display.fillRect(x+40, y, 10, 35, color);

    display.fillRect(x+3, y+53, 54, 4, 0x0000);

    display.fillTriangle(x+10, y+38, x+20, y+38, x+15, y+45, 0x0000);
    display.fillTriangle(x+40, y+38, x+50, y+38, x+45, y+45, 0x0000);

    display.fillRect(x+13, y+3, 4, 35, 0x0000);
    display.fillRect(x+43, y+3, 4, 35, 0x0000);
}

void drawMessage(Adafruit_ST7789& display, const GFXfont *font, int x, int y, int color, String message) {
    display.setFont(font);
    display.setTextColor(color);
    display.setCursor(x, y);
    display.print(message);
}