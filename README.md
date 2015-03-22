Messing with RPI and learning me some electricity.

I hope to build out a dope echo/siri like setup where I can just do various things by talking to a couple of RPI's but for now I'll just try to see if I can clap to switch speaker inputs.

Hardware Requires:
* an RPI
* an electret mic (with preamp circuit) hooked into an ADC device which inturn is hooked into the SPI pins on your RPI
* a relay hooked into your speaker and w/e number of input devicies (will describe at somepoint)

Software Requires:
* bcm2835 driver (version 1.40 works, 1.42 seems broken on arm currently)
* spi module loaded (check your /boot/config.txt to see if its enabled)

