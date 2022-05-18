
#include <iomanip>
#include <iostream>
#include <fstream>
#include <cstring>
#include <stdio.h>      // standard input / output functions
#include <unistd.h>     // UNIX standard function definitions
#include <fcntl.h>      // File control definitions
#include <errno.h>      // Error number definitions
#include <termios.h>    // POSIX terminal control definitions

#include "VeDirectFrameHandler.h"

using namespace std;

VeDirectFrameHandler veDirectFrameHandler;

/**
 * @brief Set the Socket Blocking mode
 * 
 * @param fd socket to modify
 * @param blocking true for blocking, false for non-blocking
 * @return true on success
 * @return false  on error
 */
bool SetSocketBlockingEnabled(int fd, bool blocking)
{
   if (fd < 0) return false;
   int flags = fcntl(fd, F_GETFL, 0);
   if (flags == -1) return false;
   flags = blocking ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK);
   return (fcntl(fd, F_SETFL, flags) == 0) ? true : false;
}

int main(int argc, char* argv[]) {
  string filename = "";

  for (int i = 1; i < argc; ++i) {  // i=1 because 0 is the name of the program
    if (strcmp("--serial", argv[i]) or strcmp("-S", argv[i])) {
      if (++i < argc) filename = argv[i];
    }
  }
  if (filename == "") {
    printf("Syntax:\n %s -s <serial_device>\n", argv[0]);
    return EXIT_FAILURE;
  }
  cout << endl << endl << "main() - listing on " << filename << endl;
  
  // Open the linux serial port
  int serialport = open(filename.c_str(), O_RDWR| O_NONBLOCK | O_NDELAY);
  if (serialport < 0) {
    cout << "Error " << errno << " opening " << filename << ": " << strerror(errno) << endl;
  }
  // Configure serial port
  struct termios tty;
  memset(&tty, 0, sizeof tty);
  if (tcgetattr(serialport, &tty) != 0) {
    cout << "Error " << errno << " from tcgetattr: " << strerror(errno) << endl;
  }
  // Set Baud Rate
  cfsetospeed (&tty, B19200);
  cfsetispeed (&tty, B19200);
  // Make raw
  cfmakeraw(&tty);
  // Flush Port, then applies attributes
  tcflush(serialport, TCIFLUSH);
  if (tcsetattr(serialport, TCSANOW, &tty) != 0) {
    cout << "Error " << errno << " from tcsetattr" << endl;
  }
  SetSocketBlockingEnabled(serialport, true);

  char buf = '\0';
  while(true) {
    if (read(serialport, &buf, sizeof buf) ) {
        veDirectFrameHandler.rxData(buf);
        if (veDirectFrameHandler.isDataAvailable()) {
          for (int i = 0; i < veDirectFrameHandler.veEnd; i++ ) {
            cout << std::setfill(' ') << std::setw(5) << veDirectFrameHandler.veData[i].veName;
            cout << " = " << veDirectFrameHandler.veData[i].veValue << endl;
          }
          veDirectFrameHandler.clearData();
        }
    }
  }

  cout << endl << endl;
  return EXIT_SUCCESS;
}
