
#include <iomanip>
#include <iostream>
#include <fstream>
#include <cstring>
#include <stdlib.h>
#include <stdio.h>      // standard input / output functions
#include <unistd.h>     // UNIX standard function definitions
#include <fcntl.h>      // File control definitions
#include <errno.h>      // Error number definitions
#include <termios.h>    // POSIX terminal control definitions

/*********************
 *   MQTT-c
 *********************/
#pragma GCC diagnostic ignored "-Wpedantic"
#include <MQTTClient.h>
#pragma GCC diagnostic pop
#define MQTT_TIMEOUT     10000L
#define QOS              0
#define CLIENTID         "ve2mqtt"

/*********************
 *   VeDirect
 *********************/
#include "VeDirectFrameHandler.h"
VeDirectFrameHandler veDirectFrameHandler;


using namespace std;
struct AppSettings {
  string socketPath = "";
  string mqttHost = "";
  string mqttPort = "1883";
  string mqttTopic = "";
  string mqttUsername = "";
  string mqttPassword = "";
};
AppSettings app;


void mqtt_publish(MQTTClient client, string topic, string payload) {
    MQTTClient_message message = MQTTClient_message_initializer;
    message.payload = (void *)payload.c_str();
    message.payloadlen = payload.length();
    message.qos = QOS;
    message.retained = 0;
    MQTTClient_deliveryToken token;
    MQTTClient_publishMessage(client, topic.c_str(), &message, &token);
    MQTTClient_waitForCompletion(client, token, MQTT_TIMEOUT);
}

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
  for (int i = 1; i < argc; i++) {  // i=1 because 0 is the name of the program
    if (i+1 >= argc) {
      cout << "Missing argument for parameter " << argv[i] << endl;
      continue;
    }
    if (strcmp("--serial", argv[i]) == 0 or strcmp("-s", argv[i]) == 0) app.socketPath = string(argv[++i]);
    if (strcmp("--mqtt-host", argv[i]) == 0 or strcmp("-h", argv[i]) == 0) {
      app.mqttHost = argv[++i];
      if (i+1 < argc && strncmp(argv[i+1], "-", 1) != 0) app.mqttPort = argv[++i];
    }
    if (strcmp("--mqtt-topic", argv[i]) == 0 or strcmp("-t", argv[i]) == 0) app.mqttTopic = argv[++i];
    if (strcmp("--mqtt-username", argv[i]) == 0 or strcmp("-u", argv[i]) == 0) app.mqttUsername = argv[++i];
    if (strcmp("--mqtt-password", argv[i]) == 0 or strcmp("-p", argv[i]) == 0) app.mqttPassword = argv[++i];
  }
  if (app.socketPath.empty() || app.mqttHost.empty() || app.mqttPort.empty() || app.mqttTopic.empty()) {
    cout << "Syntax:" << endl;
    cout << "\t" << argv[0] << " <options>" << endl;
    cout << "\t\t" << "-s | --serial <serial_device>" << endl;
    cout << "\t\t" << "-h | --mqtt-host <hostname|ip> [<port>]" << endl;
    cout << "\t\t" << "-t | --mqtt-topic <append_string>" << endl;
    cout << "\t\t" << "-u | --mqtt-username <username> (optional)" << endl;
    cout << "\t\t" << "-p | --mqtt-password <password> (optional)" << endl;
    cout << endl;
    return EXIT_FAILURE;
  }
  
  // Open the linux serial port
  int serialport = open(app.socketPath.c_str(), O_RDWR| O_NONBLOCK | O_NDELAY);
  if (serialport < 0) {
    cout << "Error " << errno << " opening " << app.socketPath << ": " << strerror(errno) << endl;
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

  // MQTT
  int rc;
  MQTTClient client;
  MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
  if ((rc = MQTTClient_create(&client, app.mqttHost.c_str(), CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL)) != MQTTCLIENT_SUCCESS) {
    cout << "[MQTT] Failed to create client, return code " << rc << endl;
    exit(EXIT_FAILURE);
  }
  conn_opts.keepAliveInterval = 20;
  conn_opts.cleansession = 1;
  conn_opts.username = app.mqttUsername.c_str();
  conn_opts.password = app.mqttPassword.c_str();
  if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS) {
      cout << "[MQTT] Unable to connect, return code " << rc << endl;
      return EXIT_FAILURE;
  }

  // Do the actual work
  char buf = '\0';
  while(true) {
    if (read(serialport, &buf, sizeof buf) ) {
        veDirectFrameHandler.rxData(buf);
        if (veDirectFrameHandler.isDataAvailable()) {
          for (int i = 0; i < veDirectFrameHandler.veEnd; i++ ) {
            cout << std::setfill(' ') << std::setw(5) << veDirectFrameHandler.veData[i].veName;
            cout << " = " << veDirectFrameHandler.veData[i].veValue << endl;
            // publish message to broker
            mqtt_publish(client, 
              app.mqttTopic + string("/") + veDirectFrameHandler.veData[i].veName,
              veDirectFrameHandler.veData[i].veValue
            );
          }
          veDirectFrameHandler.clearData();
        }
    }
  }

  // Clean up
  MQTTClient_disconnect(client, MQTT_TIMEOUT);
  MQTTClient_destroy(&client);

  cout << endl << endl;
  return EXIT_SUCCESS;
}
