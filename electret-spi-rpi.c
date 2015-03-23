/*
 * Read SPI values from what is assumed to be an electret mic attached to CH0
 * Detects peak sound and prints 1
 * usage: program [buffer] [threshold] [window]
 * buffer => min difference between neighbouring values
 * threshold => min value crossed to be considered spike
 * window => range of values to consider (always check midvalue of window range)
 *
 * Author: Abdi Dahir
 */

#include <bcm2835.h> //BCM stuff by Mike McCauley
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>

void sigterm(int signum) {
    bcm2835_spi_end();
    bcm2835_close();
}

int main(int argc, char **argv) {
    if (!bcm2835_init())
        return 1;

    int window_size = 64;
    int BUFFER = 100;
    int THRESH = 400;

    // arg bs
    if (argc > 1)
        BUFFER = atoi(argv[1]);

    if (argc > 2)
        THRESH = atoi(argv[2]);

    if (argc > 3)
        window_size = atoi(argv[3]);

    if (argc > 4) {
        printf("usage: %s [buffer] [threshold] [window]\n", argv[0]);
        return 1;
    }

    int g27 = open("/sys/class/gpio/gpio27/value", O_WRONLY);
    if (g27 == -1) 
        return 1;

    // She cleans up nice
    struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = sigterm;
    sigaction(SIGTERM, &action, NULL);
    sigaction(SIGINT, &action, NULL);
    sigaction(SIGQUIT, &action, NULL);

    int num_claps = 0;
    
    bcm2835_spi_begin();
    bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);                   
    bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);      
    bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_1024); 
    bcm2835_spi_chipSelect(BCM2835_SPI_CS0);                      
    bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS0, LOW);      

    // Send a byte to the slave and simultaneously read a byte back from the slave
    // If you tie MISO to MOSI, you should read back what was sent
    
    unsigned char data[3];
    int window[5] = {0, 0, 0, 0, 0};
    int response = 0;
    int toggle = 0;
    char toggle_buf[8];
    ssize_t size;
    while (THRESH) {
        data[0] = 1;  //  first byte transmitted -> start bit 
        data[1] = (1 << 7); // second byte choose channel i.e (1 0 0 0 == CH0) 4 nibbles, refer to SPI chart
        data[2] = 0; // don't care

        bcm2835_spi_transfern(data, sizeof(data));

        // byte 0 & 1 of data[1] == byte 8 & 9 of 10bit response
        response = ((data[1] << 8) & 0b1100000000) | data[2];

        window[0] = window[1];
        window[1] = window[2];
        window[2] = window[3];
        window[3] = window[4];
        window[4] = response;

        //&& ((abs(window[3] - window[2]) < BUFFER/2) || (abs(window[1] - window[2]) < BUFFER/2))) 
        if ((window[2] > THRESH) 
                && (window[1] < window[2] - BUFFER/2) && (window[3] < window[2] - BUFFER/2)
                && (window[0] < window[2] - BUFFER) && (window[4] < window[2] - BUFFER)) {
            num_claps++;

            toggle = (toggle + 1) % 2;
            size = snprintf(toggle_buf, 8, "%d", toggle);
            write(g27, toggle_buf, size);

            fprintf(stderr, "CLAP %d, response: %d %d rsp:%d %d %d\n", 
                    num_claps, window[0], window[1], window[2], window[3], window[4]);
            sleep(1);

        }

    }

    return 0;
}
