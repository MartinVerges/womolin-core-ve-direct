
#include "main.h"
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>  

#include "ArduinoJson.h"
#include "VeDirectFrameHandler.h"

using namespace std;

VeDirectFrameHandler veDirectFrameHandler;

int main() {
  printf("\n\n");
  printf("main()");
  
  // FILE* input_file = fopen("/dev/ttyS0", "r");
  FILE* input_file = fopen("testdata.txt", "r");
  if (input_file == nullptr) {
    return EXIT_FAILURE;
  }

  while (!feof(input_file)) {
    veDirectFrameHandler.rxData(getc(input_file));
    /*if (veDirectFrameHandler.veEnd > 0) {
      for (int i = 0; i < veDirectFrameHandler.veEnd; i++ ) {
        printf("%10s = %s\n", veDirectFrameHandler.veData[i].veName, veDirectFrameHandler.veData[i].veValue);
      }
    }*/
  }
  fclose(input_file);

  string output = "";
  DynamicJsonDocument doc(4096);
  for (int i = 0; i < veDirectFrameHandler.veEnd; i++ ) {
    doc[veDirectFrameHandler.veData[i].veName] = veDirectFrameHandler.veData[i].veValue;
  }
  serializeJsonPretty(doc, output);

  printf("%s", output.c_str());

  printf("\n\n");
  return EXIT_SUCCESS;
}




/* BMV 712 Smart output
Checksum	Z
H1	-171574
H2	-815
H3	-84752
H4	36
H5	0
H6	-8435205
H7	30
H8	15466
H9	2851
H10	70
H11	0
H12	0
H15	26
H16	8955
H17	10914
H18	12475
Checksum	�
PID	0xA381
V	13489
T	20
I	-310
P	-4
CE	-815
SOC	996
TTG	14400
Alarm	OFF
Relay	OFF
AR	0
BMV	712 Smart
FW	0408
MON	0
*/