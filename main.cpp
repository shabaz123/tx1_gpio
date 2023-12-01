#include "gpio.h"
#include <string.h>
#include <stdio.h>
#include <stdint.h>

int main(void) {
    reg_t regset;

    gpio_init(); // does the mmap

    tx1_gpio_name_decode("GPIO3_PJ.07", &regset); // decode the GPIO name into a structure called regset
    tx1_print_gpio_status(&regset); // print out the registers for that GPIO
    printf("setting GPIO to output mode...\n");
    tx1_gpio_set_output(&regset, regset.pin); // set that GPIO to an output
    printf("setting GPIO to a 1...\n");
    tx1_gpio_set_value(&regset, regset.pin, 1); // set that GPIO to a 1
    tx1_print_gpio_status(&regset); // print out the registers for that GPIO
    printf("done\n");

    //gpio_close();

    return 0;
}
