cmd_drivers/pinctrl/built-in.o :=  arm-linux-ld -EL    -r -o drivers/pinctrl/built-in.o drivers/pinctrl/core.o drivers/pinctrl/pinctrl-utils.o drivers/pinctrl/pinmux.o drivers/pinctrl/pinconf.o drivers/pinctrl/pinctrl-nuc980.o drivers/pinctrl/freescale/built-in.o drivers/pinctrl/nomadik/built-in.o 
