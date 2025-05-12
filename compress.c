#include <stdio.h> 
#include <stdlib.h> 
#include <math.h>

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
  {
    // find source png width and height (following spec at https://www.w3.org/TR/png-3/)
    //         init len IHDR
    int offset = 8 + 4 + 4;
    uint8_t width_arr[] = {0,0,0,0};
    uint8_t height[4] = {0,0,0,0}; // actually uint32_t
    fseek(inputptr, offset, 0);
    fread(width_arr, 1, 4, inputptr);
    uint32_t width = 0;
    for (int i = 0; i < 4; i++) {
      printf("width[%d]: %d\n",i,width_arr[i]);
      width = width + pow(2,i) * width_arr[i];
    }
    printf("width: %lu\n", (unsigned long)width);
    
    
    uint8_t header[] = {'q','o','i','f'};
    
  }

  // cleanup
  fclose(inputptr);
  fclose(outputptr);
  
  return 0;
}
