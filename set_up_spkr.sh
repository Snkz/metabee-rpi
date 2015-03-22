echo 22 > /sys/class/gpio/export
echo 27 > /sys/class/gpio/export
echo in > /sys/class/gpio/gpio22/direction
echo out > /sys/class/gpio/gpio27/direction
echo 1 > /sys/class/gpio/gpio27/value
