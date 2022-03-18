# PCF8574toMQTT
Bridge from PCF8574 I2C Port Expander to MQTT

In order to extend my home automation platform a little bit, i have ordered a I2C-Input-Module from www.horter.de. https://www.horter.de/i2c/i2c-input-modul/i2c-input-modul-Beschreibung.html

After soldering the module I wanted to check that everything works, which brought me to a little project you can find here.

This PCF8574toMQTT Gatway is a small c program that can be used for two purposes.

1. Check if the Module works and display the status of the PINs via the console. => ModeVisual
2. Get the status of the PIN of the module an publish a MQTT Message to the MQTT broker if a PIN goes High. => Normal mode

## Installation:

```
apt install build-essential mosquitto-dev
clone the source code
make
```

## Usage: Normal MQTT Mode

For the modevisual the parameter -m is required. In addition the I2C Device and the address has to be provided.

The program will connect to the I2C Bus and read the value from the given address register. If a pin goes high, a MQTT Message will be sent to the broker in a JSON string format. MQTT Message: {"Pin":"1","Status":"H"}. The program currently does not support the transmission of MQTT messages if a pin goes low. 

```
./PCF8574toMQTT -b /dev/i2c-1 -a 0x38 -h mqtt.test.com -t test/test -v

PCF8577toMQTT Bridge

Parameter
 - i2cbus     : '/dev/i2c-1'
 - address    : '0x38'
 - mqtt host  : 'mqtt.horstland.net'
 - mqtt topic : 'test/test'
 - verbose    : verbose

 Send pin status via MQTT

MQTT Message: {"Pin":"1","Status":"H"}
```

## Usage: Visual Mode

The parameter -m (ModeVisual) configures the program to output the status of the Pins.

```
./PCF8574toMQTT -b /dev/i2c-1 -a 0x38 -m

PCF8577toMQTT Bridge

Parameter
 - i2cbus     : '/dev/i2c-1'
 - address    : '0x38'

 Show pin status 

Showing status of input pins. L=Low  H=High:
Press CTRL-C to end program.

1 2 3 4 5 6 7 8
L L L H L L L L
```
## to do 

- [ ] Add feature to send a message wjem pin goes low
- [ ] Daemonize standard mode 
- [ ] Add syslog features in daemon mode
