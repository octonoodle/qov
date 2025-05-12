#include <stdio.h> 
#include <stdlib.h> 
#include <math.h>
#include <string.h>

#define INPUT_FILE "apples.png"
#define OUTPUT_FILE "apples.qoi"

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

  // BEGIN DATA ENCODING
  uint8_t seen_pix[64];
  memset(seen_pix, 0, sizeof(seen_pix));

  bool done = 0;
  while(!done) {
    sleep(1);
  }
    
  // termination

  // cleanup
  fclose(inputptr);
  fclose(outputptr);
  
  return 0;
}

int index_hash(int r, int g, int b) {
  return (r * 3 + g * 5 + b * 7) % 64;
}
int index_hash_a(int r, int g, int b, int a) {
  return (r * 3 + g * 5 + b * 7 + a * 11) % 64;
}
