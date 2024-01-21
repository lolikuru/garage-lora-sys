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
  display.drawLine(0, 7, 128, 7, SSD1306_WHITE);
}

void drawCircle(byte i){
  
  display.drawCircle( 3*(i*3+1)+90, 3, 3, SSD1306_WHITE);
  display.display();
}
