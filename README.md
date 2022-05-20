# WomoLIN.core module 

This module is meant to bridge VE.direct to MQTT.

# Installation

Clone the repository to your Raspberry pi and execute the following commands:

```
mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Release .. \
 && cmake --build . -j $(nproc) \
 && cmake --install .
```

Please edit the file `/etc/default/womolin-ve2mqtt` to fullfill your needs.
Usually you should set a topic as well as the right MQTT address.

Now to enable your service, make sure that you know the correct serial port and execute the command:

```
systemctl enable ve2mqtt@ttyS0
```
_Note_: In my example, my Ve.Direct device is connected on /dev/ttyS0 (serial port 1)._

# Running testdata instead of a ve.direct device

In order to feed test data into the process.
To do so, you have to start it up as follows:

```
export TEST_DATA=testdata/testdata-bmv712smart.txt
export TEST_MQTT_ADDRESS=192.168.254.2
export TEST_MQTT_TOPIC=vetestdata
export TEST_MQTT_USER=testuser
export TEST_MQTT_PASS=testpass
./ve2mqtt test
```

# License

rv-smart-tanksensor (c) by Martin Verges.

rv-smart-tanksensor is licensed under a Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.

You should have received a copy of the license along with this work.
If not, see <http://creativecommons.org/licenses/by-nc-sa/4.0/>.

