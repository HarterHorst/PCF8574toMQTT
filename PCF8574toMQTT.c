// PCFmqttbridge
// Version 0.9
//
// Licence information
//
// to do:
// - implement function to transmit status going from high to low for pins
// - 

#include <mosquitto.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <math.h>
#include <unistd.h>
#include <string.h>

bool optverbose = false;
char *mqtthost = 0, *mqtttopic = 0;

int isPowerOfTwo(unsigned n)
{
    return n && (!(n & (n - 1)));
}
 
int GetPinNumber(unsigned n)
{
    if (!isPowerOfTwo(n))
        return -1;
    unsigned i = 1, pos = 1;
    while (!(i & n)) {
        i = i << 1;
        ++pos;
    }
    return pos;
}
int mqttsend (int pin, int status) {
    int rc;
    struct mosquitto *mosq = NULL;
    bool clean_session = true;
    char mqttpayload[] = "{\"Pin\":\"x\",\"Status\":\"H\"}"; //mqtt payload
    mqttpayload[8] = pin + '0'; //replace with current high pin
    mqttpayload[21] = status;
    if (optverbose) {printf ("MQTT Message: %s\n",mqttpayload);}
    
    mosquitto_lib_init();
    
    mosq = mosquitto_new(NULL, clean_session, NULL);
    rc = mosquitto_connect(mosq,mqtthost, 1883, 60);
    if (rc != 0) {
        printf ("Could not connect to MQTT Broker\n");
        mosquitto_destroy (mosq);
        return -1;
    }

    mosquitto_publish(mosq, NULL, mqtttopic, strlen(mqttpayload), mqttpayload, 0, false);
    mosquitto_disconnect (mosq);
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();
    return 0;
}

int main (int argc, char **argv) {

    bool optmodevisual = false;
    char *i2cbus = 0, *i2caddress = 0;
    int c;
    int lashighpin;

    static struct option long_options[] = {
        {"i2cbus",          required_argument, NULL, 'b'},
        {"i2caddress",      required_argument, NULL, 'a'},
        {"mqtthostname",    required_argument, NULL, 'h'},
        {"mqtttopic",       required_argument, NULL, 't'},
        {"verbose",         no_argument      , NULL, 'v'},
        {"modevisual",      no_argument      , NULL, 'm'},
        {NULL,      0,                 NULL, 0}
    };

    int option_index = 0;
    char string[16] = "L L L L L L L L\0";

    //
    // Check commandline arguments
    //
    if (argc==1) {
        printf ("Error: No I2CBUS specified!\n\n");
        printf ("Usage: PCFmqttbridge [--i2cbus or -b] I2CBUS [--i2caddress or -a] ADDRESS [--mqtthostname or -h] MQTTHOSTNAME ");
        printf ("[--mqtttopic or -t] MQTTTOPIC [--modevisual or -m]\n");
        printf ("       PCFmqttbridge -b /dev/i2c-1 -a 0x38 -h 192.168.0.33 -t home/switch1\n");
        printf ("  I2CBUS is a device e.g. /dev/i2c-1.\n");
        printf ("  ADDRESS is a hex number e.g. 0x38.\n");
        printf ("  MQTTGOSTNAME is the ip address or dns hostname of the mqtt server.\n");
        printf ("  MQTTTOPIC is the MQTT topic to be send to the mqtt server.\n");
        printf ("  [--modevisual or -m] just show the status of the pins and do not send MQTT messages.\n\n");
        exit(1);
    }

    printf ("PCF8577toMQTT Bridge\n\nParameter\n");

    while ((c = getopt_long(argc, argv, "b:a:h:t:vm", long_options, &option_index)) != -1) {
        int this_option_optind = optind ? optind : 1;
        switch (c) {
        case 'b':
            printf (" - i2cbus     : '%s'\n", optarg);
            i2cbus = optarg;
            break;
        case 'a':
            printf (" - address    : '%s'\n", optarg);
            i2caddress = optarg;
            break;
        case 'h':
            printf (" - mqtt host  : '%s'\n", optarg);
            mqtthost = optarg;
            break;
        case 't':
            printf (" - mqtt topic : '%s'\n", optarg);
            mqtttopic = optarg;
            break;
        case 'v':
            printf (" - verbose    : verbose\n");
            optverbose = true;
            break;
        case 'm':
            optmodevisual = true;
            break;
        case '?':
            break;
        default:
            printf ("?? getopt returned character code 0%o ??\n", c);
        }
    }

    //
    // Create I2C bus
    //
    int file;
    if((file = open(i2cbus, O_RDWR)) < 0)
    {
        printf("Error opening the I2C Bus. Please check device name.\n");
        exit(1);
    }
    int num = (int)strtol(i2caddress, NULL, 16);   

    ioctl(file, I2C_SLAVE, num);

    // Set all pins as INPUT(0xFF)
    char config[1] = {0};
    config[0] = 0xFF;
    write(file, config, 1);

    int data_old = 255;

    if (optmodevisual) {
        printf ("\n>> Show pin status <<\n\n");

        printf("Showing status of input pins. L=Low  H=High:\n");
        printf("Press CTRL-C to end program.\n\n");
        sleep(1);

        printf ("1 2 3 4 5 6 7 8\n");
        printf ("_______________\n");
        printf ("L L L L L L L L\0");
        printf("\e[?25l"); // disable to cursor to be printed out 
        fflush(stdout);

        while (1) {
            char data[1] = {0};
            if(read(file, data, 1) != 1) {
                printf("Error : Input/output Error \n");
                exit (1);
            }
            else {
                int data1 = (data[0] & 0xFF);
                if (data1 != data_old) {
                    for(int i=0; i<8; i++) {
                        if((data1 & ((int)pow(2, i))) == 0) {
                            string[i*2] = 72; //72 is ASCII for H
                        }
                        else {
                            string[i*2] = 76; //76 is ASCII for L
                        }
                    }
                    printf ("\r%s",string);
                    printf("\e[?25l"); // disable to cursor to be printed out 

                    fflush(stdout);
                    printf("\e[?25h"); // enable cursor
                    fflush(stdout);

                    data_old = data1;
                }
            }
        }
    }
    else {
        printf ("\n>> Send pin status via MQTT <<\n\n");

        while (1) {
            char data[1] = {0};
            if(read(file, data, 1) != 1) {
                printf("Error : Input/output Error \n");
                exit (1);
            }
            else {
                // Output to screen
                int data1 = (data[0] ^ 0xFF); // invert bits
                int pinhigh = GetPinNumber (data1); // get pin numnber from inverted bits
                if (lashighpin!=pinhigh && pinhigh != -1) { 
                    // printf ("Pin: %i is high.\n", pinhigh);
                    mqttsend(pinhigh,72);
                    sleep(1);
                    }
                lashighpin=pinhigh; // save current bit status 
                }
            }
        }

exit (0);
}
