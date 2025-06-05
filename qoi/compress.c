#include <stdio.h> 
#include <stdlib.h> 
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include <unistd.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h" // not going to exist on the microcontroller

#define INPUT_FILE "test.png"
#define OUTPUT_FILE "test.qoi"



struct pixel {
  uint8_t r;
  uint8_t g;
  uint8_t b;
};

int index_hash(struct pixel pix) {
  return (pix.r * 3 + pix.g * 5 + pix.b * 7 + 255 * 11) % 64;
}
int index_hash_a(struct pixel pix, int a) {
  return (pix.r * 3 + pix.g * 5 + pix.b * 7 + a * 11) % 64;
}

bool pix_equal(struct pixel pix1, struct pixel pix2) {
  return pix1.r == pix2.r && pix1.g == pix2.g && pix1.b == pix2.b;
}

void main_decode(FILE *outputptr) {

  // data initialization
  int x,y,n,ok;
  ok = stbi_info(INPUT_FILE, &x, &y, &n);
  unsigned char *temp = stbi_load(INPUT_FILE, &x, &y, &n, 0);
  printf("x: %d, y: %d, color: %d, ok? %s\n",x,y,n, ok ? "true" : "false");

  // force 3-channel mode
  unsigned char *image;
  if (n == 4) {
    image = malloc(3*x*y*sizeof(unsigned char));
    for (int i = 0; i < x * y; i++) {
      image[3*i] = temp[4*i];
      image[3*i + 1] = temp[4*i + 1];
      image[3*i + 2] = temp[4*i + 2];
    }
  } else {
    image = stbi_load(INPUT_FILE, &x, &y, &n, 0);
  }
  stbi_image_free(temp);
  
  int firstpix = 10;
  printf("first %d pixels: ",firstpix);
  for (int i = 0; i < firstpix; i++) {
    printf("%02X%02X%02X ", image[3*i], image[3*i + 1], image[3*i + 2]);
  }
  printf("\n");
  
  struct pixel seen_pix[64];
  memset(seen_pix, 0, sizeof(seen_pix));
  struct pixel prev_pix = {0,0,0};

  // chunk encoding loop
  bool running = false;
  for(int pix = 0; pix < x * y; pix++) {
    struct pixel this_pix = {image[3*pix], image[3*pix + 1], image[3*pix + 2]};
    if (pix_equal(this_pix, prev_pix)) {
      if (running) {
	fseek(outputptr, -1, SEEK_CUR);
	uint8_t run_so_far;
	fread(&run_so_far, 1, 1, outputptr);
	run_so_far = run_so_far - 128 - 64; // get rid of header
	if (run_so_far == 61) {
	  uint8_t mt_run = 128 + 64;
	  fwrite(&mt_run, 1, 1, outputptr); // new run in next byte
	} else {
	  fseek(outputptr, -1, SEEK_CUR);
	  uint8_t new_run = run_so_far + 128 + 64 + 1;
	  fwrite(&new_run, 1, 1, outputptr); //increment run
	}
      } else {
	running = true;
	uint8_t mt_run = 128 + 64;
	fwrite(&mt_run, 1, 1, outputptr); // new run in next byte
      }
    } else { // not same as prev pix
      running = false;
      uint8_t this_hash = index_hash(this_pix);
      if (pix_equal(this_pix,seen_pix[this_hash])) { // use index
	fwrite(&this_hash, 1, 1, outputptr);
      } else { // not indexed
	seen_pix[this_hash] = this_pix; // a new pixel approaches
	int dr = this_pix.r - prev_pix.r;
	int dg = this_pix.g - prev_pix.g;
	int db = this_pix.b - prev_pix.b;
  int dr_dg = dr - dg;
  int db_dg = db - dg;
	if ((dr < 2 && dr > -3) && (dg < 2 && dg > -3) && (db < 2 && db > -3)) { // use diff [-2 to 1]
	  dr += 2;
	  dg += 2;
	  db += 2;
	  uint8_t diff = 64 + 16 * dr + 4 * dg + db;
	  fwrite(&diff, 1, 1, outputptr);
	} else if ((dr_dg < 8 && dr_dg > -9) && (dg < 32 && dg > -33) && (db_dg < 8 && db_dg > -9)) { // use luma [-8 to 7]
    uint8_t byte2 = (dr_dg + 8) * 16 + db_dg + 8;
	  uint8_t luma[2] = {128 + dg + 32, byte2};
	  fwrite(luma, 1, 2, outputptr);
	} else { // full rgb pixel
	  uint8_t rgb[4] = {254, this_pix.r, this_pix.g, this_pix.b};
	  fwrite(rgb, 1, 4, outputptr);
	}
      }
    }

    // very important!
    prev_pix = this_pix;
  }
  
  if(n == 4) {
    // image buffer loaded with stb_png, need to use devoted free
    stbi_image_free(image);
  } else {
    free(image);
  }
  
  printf("done main\n");
}

int main() 
{
  printf("welcome to auren's QOI image compressor\ncompressing file \"%s\" to output \"%s\"\n",INPUT_FILE,OUTPUT_FILE);

  // opening
  FILE *inputptr;
  inputptr = fopen(INPUT_FILE, "rb");
  FILE *outputptr;
  outputptr = fopen(OUTPUT_FILE, "wb+");

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
  //if (color_type == 2) {
  //  channels = 3;
  //} else if (color_type == 6) {
  //  channels = 4;
  //} else {
  //  printf("error, non-compatible color type [%d]", color_type);
  //  return -1;
  //}

  // populate header
  uint8_t header[14] = {'q','o','i','f',0,0,0,0,0,0,0,0,0,0};
  memcpy(header + 4*sizeof(uint8_t), width_arr, 4*sizeof(uint8_t));
  memcpy(header + 8*sizeof(uint8_t), height_arr, 4*sizeof(uint8_t));
  header[12] = 3;//channels;
  header[13] = 1; // generic RGB
  printf("the header: ");
  for (int i = 0; i < 14; i++) printf("[%x] ",header[i]);
  printf("\n");
  
  int n = sizeof(header) / sizeof(uint8_t);
  fwrite(header, sizeof(uint8_t), n, outputptr); //header

  // - - - - - - - - - - - - -
  main_decode(outputptr);
  // - - - - - - - - - - - - -
  
  // termination
  uint8_t end[8] = {0, 0, 0, 0, 0, 0, 0, 1};
  fseek(outputptr, 0, SEEK_END);
  fwrite(end, 1, 8, outputptr);
  printf("done done!\n");

  
  // cleanup
  fclose(inputptr);
  fclose(outputptr);

  
  return 0;
}
