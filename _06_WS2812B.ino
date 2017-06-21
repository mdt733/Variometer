#include <Adafruit_NeoPixel.h>

#define PIN            6
#define NUMPIXELS      3
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

long pwmx[] = {0, 64, 112, 176, 255};
long pwmy[] = {0,  5,  15,  63, 255};
static struct table_1d pwm_table = {5, pwmx, pwmy};

void spectrum(int led, int value, char x, char y, char z) //0-255 input, led * xyz / 10
{
  int r, g, b;
  g = 255 - (abs(128 - value) * 2);
  r = 255 - (abs(196 - value) * 2);
  b = 255 - (abs(64 - value) * 2);

  pixels.setPixelColor(led,
                       (int)(x * interpolate_table_1d(&pwm_table, r)) * 0.1,
                       (int)(y * interpolate_table_1d(&pwm_table, g)) * 0.1,
                       (int)(z * interpolate_table_1d(&pwm_table, b)) * 0.1);
}

void colourBar(int i)
{
  spectrum(2, i, 0, 1, 6);  //  r g b output colour range value
  spectrum(1, i, 1, 5, 1);   //  r g b output colour range value r g b
  spectrum(0, i, 5, 1, 0);   //  r g b output colour range value r g b
  pixels.show(); // This sends the updated pixel color to the hardware.
}
