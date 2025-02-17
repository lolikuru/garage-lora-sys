//void testdrawline() {
//  int16_t i;
//
//  display.clearDisplay(); // Clear display buffer
//
//  for (i = 0; i < display.width(); i += 4) {
//    display.drawLine(0, 0, i, display.height() - 1, SSD1306_WHITE);
//    display.display(); // Update screen with each newly-drawn line
//    delay(1);
//  }
//  for (i = 0; i < display.height(); i += 4) {
//    display.drawLine(0, 0, display.width() - 1, i, SSD1306_WHITE);
//    display.display();
//    delay(1);
//  }
//  delay(250);
//}
//
//void drawHeadLine() {
//  u8g2.drawLine(0, 9, 128, 9);
//}

void drawCircles(byte i, bool color) {
  if (color == true) {
    u8g2.setDrawColor(1);
    //u8g2.drawCircle( 3 * (i * 3 + 1) + 54, 3, 3, U8G2_DRAW_ALL);
  } else {
    u8g2.setDrawColor(0);
  }
  u8g2.drawCircle( 3 * (i * 3 + 1) + 54, 3, 3, U8G2_DRAW_ALL);
  u8g2.sendBuffer();
}
//
//void drawBatImage(int x, int y) {
//  //int x=54;
//  //int y=0;
//  display.drawLine(x + 1, 0, x + 2, 0, SSD1306_WHITE);
//  display.drawRect(x, 1, 4, 6, SSD1306_WHITE);
//}
//
