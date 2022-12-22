
#include <iomanip>
#include <iostream>
#include <fstream>
#include <cstring>
#include <stdlib.h>
#include <algorithm>
#include <stdio.h>      // standard input / output functions
#include <unistd.h>     // UNIX standard function definitions
#include <fcntl.h>      // File control definitions
#include <errno.h>      // Error number definitions
#include <termios.h>    // POSIX terminal control definitions
#include <csignal>      // Signal handling
#include <cmath>        // round

/*********************
 *   MQTT-c
 *********************/
#pragma GCC diagnostic ignored "-Wpedantic"
#include <MQTTClient.h>
#pragma GCC diagnostic pop
#define MQTT_TIMEOUT     10000L
#define QOS              0
MQTTClient mqttClient;

/*********************
 *   VeDirect
 *********************/
#include "VeDirectFrameHandler.h"
VeDirectFrameHandler veDirectFrameHandler;


using namespace std;
struct AppSettings {
  string socketPath = "";
  string mqttAddress = "";
  string mqttClientId = "";
  string mqttTopic = "";
  string mqttUsername = "";
  string mqttPassword = "";
};
AppSettings app;
int serialport;

void mqtt_publish(string topic, string payload) {
    MQTTClient_message message = MQTTClient_message_initializer;
    message.payload = (void *)payload.c_str();
    message.payloadlen = payload.length();
    message.qos = QOS;
    message.retained = 1;
    MQTTClient_deliveryToken token;
    MQTTClient_publishMessage(mqttClient, topic.c_str(), &message, &token);
    MQTTClient_waitForCompletion(mqttClient, token, MQTT_TIMEOUT);
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

/**
 * @brief Transform a string to uppercase
 * 
 * @param input string to convert
 * @return string in only uppercase
 */
string str_toupper(string input) {
  transform(input.begin(), input.end(), input.begin(), 
    [](unsigned char c){ return toupper(c); }
  );
  return input;
}

/**
 * @brief Print out the command syntax and exit with failure
 */
void exit_syntax() {
  cout << "Syntax:" << endl;
  cout << "\t ve2mqtt <tty>" << endl;
  cout << endl;
  cout << "Please provide the following environment variables in all upper case:" << endl;
  cout << "\t <tty>_MQTT_ADDRESS    Address of the MQTT (tcp://127.0.0.1:1883)" << endl;
  cout << "\t <tty>_MQTT_TOPIC      Topic where to store the VeDirect data" << endl;
  cout << "\t <tty>_MQTT_USER       Username for the MQTT (optional)" << endl;
  cout << "\t <tty>_MQTT_PASS       Password for the MQTT (optional)" << endl;
  cout << "\t <tty>_MQTT_CLIENT_ID  Unique client ID to connect to the MQTT" << endl;
  cout << endl;
  exit(EXIT_FAILURE);
}

/**
 * @brief Catch an interrupt signal and exit clean
 * 
 * @param signum received signal, for example SIGINT
 */
void exit_clean(int signum) {
  cout << "Interrupt signal (" << signum << ") received." << endl;

  // Disconnect from MQTT
  MQTTClient_disconnect(mqttClient, MQTT_TIMEOUT);
  MQTTClient_destroy(&mqttClient);

  // Close serial port
  close(serialport);

  cout << endl << endl;
  if (signum == SIGKILL) exit(EXIT_FAILURE);
  else exit(EXIT_SUCCESS);
}

/**
 * @brief Get environment variable content as a string
 * 
 * @param ident name of the environmen variable
 * @return string content or empty
 */
string env(string ident) {
  char const* tmp = getenv(ident.c_str());
  if ( tmp == NULL ) return string();
  return string(tmp);
}

int main(int argc, char* argv[]) {
  signal(SIGINT, exit_clean);
  signal(SIGTERM, exit_clean);
  
  if (argc != 2) exit_syntax();
  app.socketPath = string("/dev/") + string(argv[1]);
  auto arg1 = str_toupper(string(argv[1]));
  app.mqttAddress = env(arg1 + "_MQTT_ADDRESS");
  app.mqttTopic = env(arg1 + "_MQTT_TOPIC");
  app.mqttUsername = env(arg1 + "_MQTT_USER");
  app.mqttPassword = env(arg1 + "_MQTT_PASS");
  app.mqttClientId = env(arg1 + "_MQTT_CLIENT_ID");
  if (app.mqttClientId.empty()) app.mqttClientId = app.mqttUsername;
  
  if (app.mqttAddress.empty() || app.mqttTopic.empty() || app.mqttClientId.empty()) exit_syntax();

  string testdata = env(string("TEST_DATA"));
  if (testdata.length()) {
    // Feed testdata
    cout << "Running with test data " << testdata << endl;
    serialport = open(testdata.c_str(), O_RDONLY);
    if (serialport < 0) {
      cerr << "[ERROR] Code " << errno << " opening " << testdata << ": " << strerror(errno) << endl;
      exit(EXIT_FAILURE);
    }
  } else {
    // Use a socket
    cout << "Binding to " << app.socketPath << " and listen for Ve.Direct messages." << endl;

    // Open the linux serial port
    serialport = open(app.socketPath.c_str(), O_RDONLY);
    if (serialport < 0) {
      cerr << "[ERROR] Code " << errno << " opening " << app.socketPath << ": " << strerror(errno) << endl;
      exit(EXIT_FAILURE);
    }
    // Configure serial port
    struct termios tty;
    memset(&tty, 0, sizeof tty);
    if (tcgetattr(serialport, &tty) != 0) {
      cerr << "[ERROR] Code " << errno << " from tcgetattr: " << strerror(errno) << endl;
    }
    // Set Baud Rate
    cfsetospeed (&tty, B19200);
    cfsetispeed (&tty, B19200);
    // Make raw
    cfmakeraw(&tty);
    // Flush Port, then applies attributes
    tcflush(serialport, TCIFLUSH);
    if (tcsetattr(serialport, TCSANOW, &tty) != 0) {
      cerr << "[ERROR] Code " << errno << " from tcsetattr" << endl;
    }
    SetSocketBlockingEnabled(serialport, true);
  }

  // MQTT
  int rc;
  MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
  if ((rc = MQTTClient_create(&mqttClient, app.mqttAddress.c_str(), app.mqttClientId.c_str(), MQTTCLIENT_PERSISTENCE_NONE, NULL)) != MQTTCLIENT_SUCCESS) {
    cerr << "[MQTT] Failed to create client, return code " << rc << endl;
    exit(EXIT_FAILURE);
  } else  {
    cout << "[MQTT] Created a client with ID " << app.mqttClientId << " for Broker address " << app.mqttAddress << endl;
  }
  conn_opts.keepAliveInterval = 20;
  conn_opts.cleansession = 1;
  if (app.mqttUsername.length()) conn_opts.username = app.mqttUsername.c_str();
  if (app.mqttPassword.length()) conn_opts.password = app.mqttPassword.c_str();
  if ((rc = MQTTClient_connect(mqttClient, &conn_opts)) != MQTTCLIENT_SUCCESS) {
      cerr << "[MQTT] Unable to connect, return code " << rc << endl;
      cerr << "[MQTT] Hint: Check the username and password or take a look at the mqtt broker logs" << endl;
      return EXIT_FAILURE;
  } else {
    cout << "[MQTT] Broker connection established!" << endl;
  }

  // Do the actual work
  char buf = '\0';
  while(true) {
    if (read(serialport, &buf, sizeof buf) ) {
        veDirectFrameHandler.rxData(buf);
        if (veDirectFrameHandler.isDataAvailable()) {
          for (int i = 0; i < veDirectFrameHandler.veEnd; i++ ) {
            // cout << "[DATA] received " << veDirectFrameHandler.veData[i].veName << " with Value " << veDirectFrameHandler.veData[i].veValue << endl;
            if (!testdata.empty()) {
              cout << std::setfill(' ') << std::setw(5) << veDirectFrameHandler.veData[i].veName;
              cout << " = " << veDirectFrameHandler.veData[i].veValue << endl;
              usleep(100000);
            }
            // publish message to broker
            if (strcmp(veDirectFrameHandler.veData[i].veName, "SER#") == 0) {
              // FIXME: Deal better than skipping on special chars
              cout << "[MQTT] Skipping illegal topic name ser#" << endl;
            } else {
              if (strcmp(veDirectFrameHandler.veData[i].veName, "SOC") == 0) {
                float level = (float)atoi(veDirectFrameHandler.veData[i].veValue) / 10.f;
                mqtt_publish(app.mqttTopic + string("/level"), to_string((int)round(level)).c_str());
              }
              mqtt_publish(app.mqttTopic + string("/") + veDirectFrameHandler.veData[i].veName,
                veDirectFrameHandler.veData[i].veValue
              );
            }
          }
          veDirectFrameHandler.clearData();
        }
    }
  }
  exit_clean(SIGKILL);
}
