cmd_drivers/gpio/built-in.o :=  arm-linux-ld -EL    -r -o drivers/gpio/built-in.o drivers/gpio/devres.o drivers/gpio/gpiolib.o drivers/gpio/gpiolib-legacy.o drivers/gpio/gpiolib-sysfs.o drivers/gpio/gpio-nuc980.o 