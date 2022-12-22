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

## Serial Ports of the Raspberry Pi

If you want to connect multiple Victron Energy ve.direct devices to the RPi, you have to enable additional UARTs.
Depending on the model, you have up to 5 UART Serial Ports available. 

Edit `/boot/config.txt` and add the correspondig dtoverlay for the desired port. 
```
# dtoverlay=uart1       # GPIO 14/15
# dtoverlay=uart2       # GPIO 0/1
# dtoverlay=uart3       # GPIO 4/5
# dtoverlay=uart4       # GPIO 8/9
# dtoverlay=uart5       # GPIO 12/13
``` 
_Note_: You have to reboot the RPi to apply this new configuration. Beware, it is GPIO not PIN.


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

# Using it with Home Assistant

To use the data of the VE Direct devices in the best possible way from the MQTT in HomeAssistant, here is an example configuration.
Please put it into your `configuration.yaml` file.

```
mqtt:
  sensor:
# ve2mqtt - BMS702 values
    - name: " Battery Level"
      unique_id: "batterylevel"
      state_topic: "vedirect/bms/level"
      unit_of_measurement: "%"
      icon: "mdi:car-battery"

    - name: " Battery Temperature"
      unique_id: "batterytemp"
      state_topic: "vedirect/bms/T"
      unit_of_measurement: "°C"
      icon: "mdi:temperature-celsius"

    - name: " Battery current"
      unique_id: "batterycurrent"
      state_topic: "vedirect/bms/I"
      unit_of_measurement: "mA"
      icon: "mdi:current-dc"

    - name: " Battery watts"
      unique_id: "batterywatts"
      state_topic: "vedirect/bms/P"
      unit_of_measurement: "W"
      icon: "mdi:lightning-bolt"

    - name: " Battery voltage"
      unique_id: "batteryvoltage"
      state_topic: "vedirect/bms/V"
      unit_of_measurement: "mV"
      icon: "mdi:flash-triangle"

# ve2mqtt - MPPT values
    - name: "MPPT solar charge"
      unique_id: "mpptcharge"
      state_topic: "vedirect/mppt/I"
      unit_of_measurement: "mA"
      icon: "mdi:current-dc"

    - name: "MPPT solar panel volts"
      unique_id: "mpptpvvolts"
      state_topic: "vedirect/mppt/VPV"
      unit_of_measurement: "mV"
      icon: "mdi:flash-triangle"

    - name: "MPPT solar panel power"
      unique_id: "mpptpvpower"
      state_topic: "vedirect/mppt/PPV"
      unit_of_measurement: "W"
      icon: "mdi:lightning-bolt"

```

Your dashboard could look like that:

```
    - type: vertical-stack
      cards:
        - type: gauge
        severity:
            green: 60
            yellow: 20
            red: 0
        needle: false
        name: Battery
        entity: sensor.battery_level
        - hours_to_show: 24
        graph: line
        type: sensor
        detail: 2
        entity: sensor.battery_level
        name: SoC
        - show_name: false
        show_icon: true
        show_state: true
        type: glance
        entities:
            - entity: sensor.battery_temperature
            - entity: sensor.battery_voltage
            - entity: sensor.battery_watts
            - entity: sensor.battery_current
        title: Battery
        - show_name: false
        show_icon: true
        show_state: true
        type: glance
        entities:
            - entity: sensor.mppt_solar_charge
            - entity: sensor.mppt_solar_panel_power
            - entity: sensor.mppt_solar_panel_volts
        title: Photovoltaik
        state_color: false
```

# License

ve2mqtt (c) by Martin Verges.

ve2mqtt is licensed under a Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.

You should have received a copy of the license along with this work.
If not, see <http://creativecommons.org/licenses/by-nc-sa/4.0/>.

