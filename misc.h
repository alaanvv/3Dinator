#define ASSERT(x, str) if (!(x)) {printf(str); exit(1);}

void create_circle(float x, float y, int8_t fragments, float rad, float buffer[]) {
  for (uint8_t i = 0; i < fragments; i++) {
    buffer[i * 9] = x;
    buffer[i * 9 + 1] = y;

    buffer[i * 9 + 3] = x + rad * sin(3.14159 * 2 / fragments * i);
    buffer[i * 9 + 4] = y + rad * cos(3.14159 * 2 / fragments * i);

    buffer[i * 9 + 6] = x + rad * sin(3.14159 * 2 / fragments * (i + 1));
    buffer[i * 9 + 7] = y + rad * cos(3.14159 * 2 / fragments * (i + 1));
  }
}

void create_rect(float x, float y, float w, float h, float buffer[]) {
  buffer[0] = x;
  buffer[1] = y;
  buffer[3] = x;
  buffer[4] = y + h;
  buffer[6] = x + w;
  buffer[7] = y;

  buffer[9]  = x + w;
  buffer[10] = y + h;
  buffer[12] = x;
  buffer[13] = y + h;
  buffer[15] = x + h;
  buffer[16] = y;
}

void get_image_size(char path[], uint16_t* width, uint16_t* height) {
  FILE* img = fopen(path, "r");
  ASSERT(img != NULL, "Can't open image");

  fscanf(img, "%*s %hi %hi", width, height);

  fclose(img);
}

void load_image(char path[], float buffer[], uint32_t pixels) {
  FILE* img = fopen(path, "r");
  ASSERT(img != NULL, "Can't open image");
  fscanf(img, "%*s %*i %*i %*s");

  int pixel[3];
  for (int i = 0; i < pixels; i++) {
    fscanf(img, "%i %i %i", &pixel[0], &pixel[1], &pixel[2]);
    buffer[i * 3 + 0] = (float) pixel[0] / 255;
    buffer[i * 3 + 1] = (float) pixel[1] / 255;
    buffer[i * 3 + 2] = (float) pixel[2] / 255;
  }

  fclose(img);
}
