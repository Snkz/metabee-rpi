#include <bcm2835.h> //BCM stuff by Mike McCauley
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

void sigterm(int signum) {
    bcm2835_spi_end();
    bcm2835_close();
}

int main(int argc, char **argv) {
    if (!bcm2835_init())
        return 1;

    // She cleans up nice
    struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = sigterm;
    sigaction(SIGTERM, &action, NULL);
    sigaction(SIGINT, &action, NULL);
    sigaction(SIGQUIT, &action, NULL);

    int BUFFER = 100;
    int THRESH = 400;
    int num_claps;
    //if (argc < 3)
    //    return 1;
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
    while (THRESH) {
        data[0] = 1;  //  first byte transmitted -> start bit 
        data[1] = (1 << 7); // second byte choose channel i.e (1 0 0 0 == CH0) 4 nibbles, refer to SPI chart
        data[2] = 0; // don't care

        bcm2835_spi_transfern(data, sizeof(data));

        response = ((data[1] << 8) & 0b1100000000) | (data[2] & 0b11111111);

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
            printf("CLAP %d, response: %d %d rsp:%d %d %d\n", 
                    num_claps, window[0], window[1], window[2], window[3], window[4]);

            window[0] = window[1] = window[2] = window[3] = window[4] = 0;
            usleep(100);
        }

    }

    return 0;
}
