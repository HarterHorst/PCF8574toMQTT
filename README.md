# PCF8574toMQTT
Bridge from PCF8574 I2C Port Expander to MQTT

In order to extend my home automation platform a little bit, i have ordered a I2C-Input-Module from www.horter.de. https://www.horter.de/i2c/i2c-input-modul/i2c-input-modul-Beschreibung.html

After soldering the module i wanted to check if everything works, which brought me to a little project you can find here.

This PCF8574toMQTT Gatway is a small c program that can be used for two purposes.

1. Check if the Module works and display the status of the PINs via the console
2. Get the status of the PIN of the module an publish a MQTT Message to the MQTT broker if a PIN goes High.

Installation:

...


Usage:

For the modevisual the parameter -m is required. In addition the I2C Device and the address has to be provided.

./PCF8574toMQTT -m -b /dev/i2c-1 -a 0x38

The program will connect to the I2C Bus and read the value from the given address register.


