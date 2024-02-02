void testdrawline() {
  int16_t i;

  display.clearDisplay(); // Clear display buffer

  for(i=0; i<display.width(); i+=4) {
    display.drawLine(0, 0, i, display.height()-1, SSD1306_WHITE);
    display.display(); // Update screen with each newly-drawn line
    delay(1);
  }
  for(i=0; i<display.height(); i+=4) {
    display.drawLine(0, 0, display.width()-1, i, SSD1306_WHITE);
    display.display();
    delay(1);
  }
  delay(250);
}

void drawHeadLine(){
  display.drawLine(0, 9, 128, 9, SSD1306_WHITE);
}

void drawCircles(byte i, bool color){
  if(color==true){
    display.drawCircle( 3*(i*3+1)+90, 3, 3, SSD1306_WHITE);
  } else display.drawCircle( 3*(i*3+1)+90, 3, 3, SSD1306_BLACK);
  display.display();
}

void drawSymbol(uint16_t x, uint16_t y, uint8_t c, uint16_t color, uint16_t bg, uint8_t Size){
  if( (c>=32) && (c<=126) ){ //If it's 33-126 then use standard mono 18 font
      //display.setFont(&FreeMono18pt7b);
  } else {
    //display.setFont(&SymbolMono18pt7b);//Otherwise use special symbol font
    if (c>126) {      //Remap anything above 126 to be in the range 32 and upwards
      c-=(127-32);
    }
  }
  display.drawChar(x,y,c,color,bg,Size);
}
