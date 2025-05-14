#include <stdio.h> 
#include <stdlib.h> 
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include <unistd.h>

#include "lodepng.h" // not going to exist on the microcontroller

#define INPUT_FILE "apples.png"
#define OUTPUT_FILE "apples.qoi"

struct pixel {
  uint8_t r;
  uint8_t g;
  uint8_t b;
};

int index_hash(struct pixel pix) {
  return (pix.r * 3 + pix.g * 5 + pix.b * 7) % 64;
}
int index_hash_a(struct pixel pix, int a) {
  return (pix.r * 3 + pix.g * 5 + pix.b * 7 + a * 11) % 64;
}

bool pix_equal(struct pixel pix1, struct pixel pix2) {
  return pix1.r == pix2.r && pix1.g == pix2.g && pix1.b == pix2.b;
}

void main_decode(FILE *outputptr) {

  // data initialization
  unsigned error;
  unsigned char* image = 0;
  unsigned width, height;

  error = lodepng_decode24_file(&image, &width, &height, INPUT_FILE);
  if(error) printf("error %u: %s\n", error, lodepng_error_text(error));

  struct pixel seen_pix[64];
  memset(seen_pix, 0, sizeof(seen_pix));
  struct pixel prev_pix = {0,0,0};
  
  // chunk encoding loop
  for(int pix = 0; pix < width * height; pix++) {
    struct pixel this_pix = {image[3*pix], image[3*pix + 1], image[3*pix + 2]};
    if (pix_equal(this_pix, prev_pix)) {
      printf("hoo boy");
    }
  }
  
  //printf("\n");
  //for (int j = 0; j <40; j+=4) {
  //  int i = j + (4 * width * height / 2) + 13300;
  //  printf("pixel %d: %02X%02X%02X\n",i/3,image[i],image[i+1],image[i+2]);
  //}
  
  free(image);
  printf("done");
}

int main() 
{
  printf("welcome to auren's QOI image compressor\ncompressing file \"%s\" to output \"%s\"\n",INPUT_FILE,OUTPUT_FILE);

  // opening
  FILE *inputptr;
  inputptr = fopen(INPUT_FILE, "rb");
  FILE *outputptr;
  outputptr = fopen(OUTPUT_FILE, "wb");

  if (inputptr == NULL) {
    printf("could not open input file %s\n", INPUT_FILE);
  } else {
    printf("successfully opened input file %s\n", INPUT_FILE);
  }

  if (inputptr == NULL) {
    printf("could not open output file %s\n", OUTPUT_FILE);
  } else {
    printf("successfully opened output file %s\n", OUTPUT_FILE);
  }

  // headers
  //fprintf(outputptr, "hi im in the qoi output! :)\n");
  // header [14 bytes]: qoif [4] width-px [4] height-px [4] channels (3) [1] colorspace (1) [1]
  // find source png width and height (following spec at https://www.w3.org/TR/png-3/)
  //         init len IHDR
  int offset = 8 + 4 + 4;
  uint8_t width_arr[4] = {0,0,0,0};
  uint8_t height_arr[4] = {0,0,0,0};
  fseek(inputptr, offset, SEEK_CUR);
  fread(width_arr, 1, 4, inputptr);
  //uint32_t width = 0;
  //for (int i = 0; i < 4; i++) {
  //  uint32_t coefficient = pow(2,8*(3-i)); // byte value offset
  //  width = width + coefficient * width_arr[i];
  //}
  //printf("width: %lu\n", (unsigned long)width);
  //
  fread(height_arr, 1, 4, inputptr);
  //uint32_t height = 0;
  //for (int i = 0; i < 4; i++) {
  //  uint32_t coefficient = pow(2,8*(3-i)); // byte value offset
  //  height = height + coefficient * height_arr[i];
  //}
  //printf("height: %lu\n", (unsigned long)height);

  uint8_t color_type;
  fseek(inputptr, 1, SEEK_CUR);
  fread(&color_type, 1, 1, inputptr);
  uint8_t channels;
  if (color_type == 2) {
    channels = 3;
  } else if (color_type == 6) {
    channels = 4;
  } else {
    printf("error, non-compatible color type [%d]", color_type);
    return -1;
  }

  // populate header
  uint8_t header[14] = {'q','o','i','f'};
  memcpy(header + 4*sizeof(uint8_t), width_arr, 4*sizeof(uint8_t));
  memcpy(header + 8*sizeof(uint8_t), height_arr, 4*sizeof(uint8_t));
  header[12] = channels;
  header[13] = 0; // assume sRGB
  printf("the header: ");
  for (int i = 0; i < 14; i++) printf("[%x] ",header[i]);

  int n = sizeof(header) / sizeof(uint8_t);
  fwrite(header, sizeof(uint8_t), n, outputptr); //header

  main_decode(outputptr);
  
  // termination

  // cleanup
  fclose(inputptr);
  fclose(outputptr);
  
  return 0;
}
