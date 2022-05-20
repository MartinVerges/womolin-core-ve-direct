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


# License

rv-smart-tanksensor (c) by Martin Verges.

rv-smart-tanksensor is licensed under a Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.

You should have received a copy of the license along with this work.
If not, see <http://creativecommons.org/licenses/by-nc-sa/4.0/>.

