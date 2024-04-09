#ifndef LCD_DISPLAY_H
#define LCD_DISPLAY_H

#include <LiquidCrystal_I2C.h>
#include "config.h" // Assuming LCD_ADDRESS, LCD_COLS, LCD_ROWS are defined here

#define LCD_ADDR 0x27
#define LCD_COLS 16
#define LCD_ROWS 2


class LcdDisplay {
    
public:
    static LcdDisplay& Get();
    void init();
    void displayMessage(const String& message, int col, int row);
    void clear();


private:
    LcdDisplay(); // Private constructor
    LiquidCrystal_I2C lcd;
    // Prevent copying and assignment
    LcdDisplay(const LcdDisplay&) = delete;
    LcdDisplay& operator=(const LcdDisplay&) = delete;
};

extern LcdDisplay& lcd;
#endif
