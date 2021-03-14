#ifndef _ILI9341_H_
#define _ILI9341_H_

#define SPI_PORT spi0
#define PIN_MISO 14
#define PIN_CS   13
#define PIN_SCK  6
#define PIN_MOSI 7
#define PIN_DC 15
#define PIN_RST 14
#define PIN_LED 25
#define SPI_BAUD_RATE  70000000 


// Color definitions
#define ILI9341_WHITE 0xFFFF 
#define ILI9341_BLACK 0x0000      
#define ILI9341_RED   0x02F0     
#define ILI9341_GREEN 0xC027      
#define ILI9341_BLUE 0x1F18 //0x2619
#define ILI9341_SKYBLUE 0x029E 
#define ILI9341_ORANGE 0x71E3
#define ILI9341_YELLOW 0x46EC


#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 320
#define SCREEN_TOTAL_PIXELS SCREEN_WIDTH * SCREEN_HEIGHT
#define BUFFER_SIZE SCREEN_TOTAL_PIXELS * 2

#define MADCTL_MY 0x80  // Bottom to top
#define MADCTL_MX 0x40  // Right to left
#define MADCTL_MV 0x20  // Reverse Mode
#define MADCTL_ML 0x10  // LCD refresh Bottom to top
#define MADCTL_RGB 0x00 // Red-Green-Blue pixel order
#define MADCTL_BGR 0x08 // Blue-Green-Red pixel order
#define MADCTL_MH 0x04  // LCD refresh right to left

void init_SPI();
void init_display();
void init_drawing();
void clear_buffer();
void write_buffer();
void draw_rectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);

#endif //_ILI9341_H_
