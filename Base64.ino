const char PROGMEM b64_alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

int base64_index(char c) {
  for (int i = 0; i < 64; i++) {
    if (pgm_read_byte(&b64_alphabet[i]) == c) {
      return i;
    }
  }
  return -1;
}

String base64_decode(String input) {
  String output = "";
  int i = 0, j = 0;
  uint8_t byteArray3[3];
  uint8_t byteArray4[4];

  while (input.length() > i && input[i] != '=') {
    byteArray4[j++] = input[i++];
    if (j == 4) {
      for (j = 0; j < 4; j++) {
        byteArray4[j] = base64_index((char)byteArray4[j]);
      }

      byteArray3[0] = (byteArray4[0] << 2) + ((byteArray4[1] & 0x30) >> 4);
      byteArray3[1] = ((byteArray4[1] & 0x0f) << 4) + ((byteArray4[2] & 0x3c) >> 2);
      byteArray3[2] = ((byteArray4[2] & 0x03) << 6) + byteArray4[3];

      for (j = 0; j < 3; j++) {
        output += (char)byteArray3[j];
      }
      j = 0;
    }
  }

  if (j) {
    for (int k = j; k < 4; k++) {
      byteArray4[k] = 0;
    }

    for (int k = 0; k < 4; k++) {
      byteArray4[k] = base64_index((char)byteArray4[k]);
    }

    byteArray3[0] = (byteArray4[0] << 2) + ((byteArray4[1] & 0x30) >> 4);
    byteArray3[1] = ((byteArray4[1] & 0x0f) << 4) + ((byteArray4[2] & 0x3c) >> 2);
    byteArray3[2] = ((byteArray4[2] & 0x03) << 6) + byteArray4[3];

    for (int k = 0; k < j - 1; k++) {
      output += (char)byteArray3[k];
    }
  }

  return output;
}
