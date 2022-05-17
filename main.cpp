
#include "main.h"
#include <iostream>
#include <fstream>
#include <cstring>  

#include "VeDirectFrameHandler.h"

using namespace std;

VeDirectFrameHandler veDirectFrameHandler;

int main() {
  printf("\n\n");
  printf("main()");
  
  // FILE* input_file = fopen("/dev/ttyS0", "r");
  FILE* input_file = fopen("testdata.txt", "rb");
  if (input_file == nullptr) {
    return EXIT_FAILURE;
  }

  while (!feof(input_file)) {
    veDirectFrameHandler.rxData(getc(input_file));
    if (veDirectFrameHandler.isDataAvailable()) {
      for (int i = 0; i < veDirectFrameHandler.veEnd; i++ ) {
        printf("%10s = %s\n", veDirectFrameHandler.veData[i].veName, veDirectFrameHandler.veData[i].veValue);
      }
      veDirectFrameHandler.clearData();
    }
  }
  fclose(input_file);


  printf("\n\n");
  return EXIT_SUCCESS;
}
