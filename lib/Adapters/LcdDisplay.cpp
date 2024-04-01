#include "LcdDisplay.h"




LcdDisplay::LcdDisplay() : lcd(LCD_ADDRESS, LCD_COLS, LCD_ROWS) {}

LcdDisplay& LcdDisplay::Get() {
    static LcdDisplay instance; // Guaranteed to be created only once
    return instance;
}

void LcdDisplay::init() {
    lcd.init();
    lcd.backlight();
    
}

void LcdDisplay::clear(){
    lcd.clear();
}

void LcdDisplay::displayMessage(const String& message, int col, int row) {
    lcd.setCursor(col, row);
    lcd.print(message);
}
