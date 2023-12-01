//
// Created by shabaz on 30/11/2023.
//

#include "gpio.h"
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>
#include <unistd.h>


#define GPIO_DBG 1

uint32_t GPIO_BASE_ADDR[8] = {GPIO_1, GPIO_2, GPIO_3, GPIO_4, GPIO_5, GPIO_6, GPIO_7, GPIO_8};
uint32_t GPIO_CNF_OFFSET[4] = {GPIO_CNF_0, GPIO_CNF_1, GPIO_CNF_2, GPIO_CNF_3};
uint32_t GPIO_MSK_CNF_OFFSET[4] = {GPIO_MSK_CNF_0, GPIO_MSK_CNF_1, GPIO_MSK_CNF_2, GPIO_MSK_CNF_3};
uint32_t GPIO_OE_OFFSET[4] = {GPIO_OE_0, GPIO_OE_1, GPIO_OE_2, GPIO_OE_3};
uint32_t GPIO_MSK_OE_OFFSET[4] = {GPIO_MSK_OE_0, GPIO_MSK_OE_1, GPIO_MSK_OE_2, GPIO_MSK_OE_3};
uint32_t GPIO_OUT_OFFSET[4] = {GPIO_OUT_0, GPIO_OUT_1, GPIO_OUT_2, GPIO_OUT_3};
uint32_t GPIO_MSK_OUT_OFFSET[4] = {GPIO_MSK_OUT_0, GPIO_MSK_OUT_1, GPIO_MSK_OUT_2, GPIO_MSK_OUT_3};
uint32_t GPIO_IN_OFFSET[4] = {GPIO_IN_0, GPIO_IN_1, GPIO_IN_2, GPIO_IN_3};

void* gpio_mapped;


int
gpio_init(void) {
    int fd;

    fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (fd < 0) {
        if (GPIO_DBG) printf("Error: cannot open /dev/mem, maybe try running as root user\n");
        return (-1);
    }
    // map all of GPIO1..GPIO8 into a single block
    gpio_mapped = mmap(0, GPIO_MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, GPIO_BASE);
    if (gpio_mapped == MAP_FAILED) {
        if (GPIO_DBG) printf("Error: cannot map GPIO registers\n");
        close(fd);
        return (-1);
    } else {
        if (GPIO_DBG) printf("GPIO registers mapped to %p\n", gpio_mapped);
    }

    close(fd);
    return(0);
}

int
gpio_close(void) {
    int ret;

    ret = munmap(gpio_mapped, GPIO_MAP_SIZE);
    if (ret < 0) {
        if (GPIO_DBG) printf("Error: cannot unmap GPIO registers\n");
        return (-1);
    }

    return(0);
}

int
tx1_gpio_name_decode(const char *gpio_name, reg_t *regset) {
    uint32_t gpio_cnf_addr;
    uint32_t gpio_msk_cnf_addr;
    uint32_t gpio_oe_addr;
    uint32_t gpio_msk_oe_addr;
    uint32_t gpio_out_addr;
    uint32_t gpio_msk_out_addr;
    uint32_t gpio_in_addr;

    char highport = 0; // this is set to 1 if the port is double-character, i.e. AA-EE
    int controller_num; // this is the controller number, 1-8
    int port_num; // this is the port number, 0-3
    int portchar_num; // this is the port character (A-Z or AA-EE) as a number, i.e. 0-25 or 26-30
    int pin; // this is the pin number 0-7
    // gpio_name is a string like GPIO3_PC.03
    // sanity check the string
    if (strlen(gpio_name) < 10) {
        if (GPIO_DBG) printf("Error: gpio_name is too short\n");
        return (-1);
    }
    if (gpio_name[0] != 'G' || gpio_name[1] != 'P' || gpio_name[2] != 'I' || gpio_name[3] != 'O') {
        if (GPIO_DBG) printf("Error: gpio_name does not start with GPIO\n");
        return (-1);
    }
    if (gpio_name[4] < '3' || gpio_name[4] > '3') {
        if (GPIO_DBG) printf("Error: gpio_name does not start with GPIO3\n");
        return (-1);
    }
    if (gpio_name[5] != '_') {
        if (GPIO_DBG) printf("Error: gpio_name does not have an underscore after GPIO3\n");
        return (-1);
    }
    if (gpio_name[8] >= 'A' && gpio_name[8] <= 'E') {
        // this is a double-character port name, e.g. GPIO3_PAA.03
        if (gpio_name[7] < 'A' || gpio_name[7] > 'E') {
            if (GPIO_DBG) printf("Error: gpio_name does not contain a valid port character A-Z or AA-EE\n");
            return (-1);
        }
        highport = 1;
    } else if (gpio_name[8] != '.') {
        if (GPIO_DBG) printf("Error: gpio_name does not contain a valid port character A-Z or AA-EE\n");
        return (-1);
    }
    if (highport == 0) {
        if (gpio_name[7] < 'A' || gpio_name[7] > 'Z') {
            if (GPIO_DBG) printf("Error: gpio_name does not contain a valid port character A-Z\n");
            return (-1);
        }
    }
    // calculate the portchar_num
    if (highport == 0) {
        portchar_num = gpio_name[7] - 'A';
    } else {
        portchar_num = (gpio_name[7] - 'A') + 26;
    }

    // calculate the controller_num
    controller_num = (portchar_num / 4) + 1;
    // calculate the port_num
    port_num = portchar_num % 4;
    if (GPIO_DBG) printf("gpio_name=%s, controller_num[1..8]=%d, port_num[0..3]=%d\n", gpio_name, controller_num, port_num);

    gpio_cnf_addr = GPIO_BASE_ADDR[controller_num - 1] + GPIO_CNF_OFFSET[port_num];
    gpio_msk_cnf_addr = GPIO_BASE_ADDR[controller_num - 1] + GPIO_MSK_CNF_OFFSET[port_num];
    gpio_oe_addr = GPIO_BASE_ADDR[controller_num - 1] + GPIO_OE_OFFSET[port_num];
    gpio_msk_oe_addr = GPIO_BASE_ADDR[controller_num - 1] + GPIO_MSK_OE_OFFSET[port_num];
    gpio_out_addr = GPIO_BASE_ADDR[controller_num - 1] + GPIO_OUT_OFFSET[port_num];
    gpio_msk_out_addr = GPIO_BASE_ADDR[controller_num - 1] + GPIO_MSK_OUT_OFFSET[port_num];
    gpio_in_addr = GPIO_BASE_ADDR[controller_num - 1] + GPIO_IN_OFFSET[port_num];

    regset->gpio_cnf_addr = gpio_cnf_addr;
    regset->gpio_msk_cnf_addr = gpio_msk_cnf_addr;
    regset->gpio_oe_addr = gpio_oe_addr;
    regset->gpio_msk_oe_addr = gpio_msk_oe_addr;
    regset->gpio_out_addr = gpio_out_addr;
    regset->gpio_msk_out_addr = gpio_msk_out_addr;
    regset->gpio_in_addr = gpio_in_addr;
    // also store the controller, port and pin number
    pin = gpio_name[strlen(gpio_name)-1] - '0';
    if (pin < 0 || pin > 7) {
        if (GPIO_DBG) printf("Error: gpio_name does not contain a valid pin number 0-7\n");
        return (-1);
    }
    regset->controller = controller_num;
    regset->port = port_num;
    regset->pin = pin;
    if (highport==0) {
        regset->portchar = gpio_name[7];
    } else {
        regset->portchar = '-'; // don't support AA-EE yet
    }

    if (GPIO_DBG) {
        printf("controller_num[1..8]=%d\n", controller_num);
        printf("port_num[0..3]=%d\n", port_num);
        printf("pin[0..7]=%d\n", pin);
        printf("gpio_cnf_addr=0x%08x\n", gpio_cnf_addr);
        printf("gpio_msk_cnf_addr=0x%08x\n", gpio_msk_cnf_addr);
        printf("gpio_oe_addr=0x%08x\n", gpio_oe_addr);
        printf("gpio_msk_oe_addr=0x%08x\n", gpio_msk_oe_addr);
        printf("gpio_out_addr=0x%08x\n", gpio_out_addr);
        printf("gpio_msk_out_addr=0x%08x\n", gpio_msk_out_addr);
        printf("gpio_in_addr=0x%08x\n", gpio_in_addr);
    }

    return(0);
}

int
tx1_gpio_set_output(reg_t *regset, char pin) {
    uint32_t gpio_msk_cnf_addr;
    uint32_t gpio_msk_oe_addr;
    uint16_t data;
    volatile uint16_t* reg;

    gpio_msk_cnf_addr = regset->gpio_msk_cnf_addr;
    gpio_msk_oe_addr = regset->gpio_msk_oe_addr;

    // configure the pin as GPIO, in case it isn't already done.
    // create the mask in the upper 8 bits
    data = 0x0100 << pin;
    // set the lower bit to 1 to configure as GPIO
    data |= (1 << pin);

    // write the data to the gpio_msk_cnf_addr
    if (GPIO_DBG) printf("writing 0x%04x to gpio_msk_cnf_addr 0x%08x\n", data, gpio_msk_cnf_addr);
    reg = (uint16_t *)(gpio_mapped) + (gpio_msk_cnf_addr - GPIO_BASE);
    *reg = data;
    // configure the pin to be in output mode
    // we can use the same data as above, just change the address
    if (GPIO_DBG) printf("writing 0x%04x to gpio_msk_oe_addr 0x%08x\n", data, gpio_msk_oe_addr);
    reg = (uint16_t *)(gpio_mapped) + (gpio_msk_oe_addr - GPIO_BASE);
    *reg = data;

    return(0);
}

int
tx1_gpio_set_value(reg_t *regset, char pin, char val) {
    uint32_t gpio_msk_out_addr;
    uint16_t data;
    volatile uint16_t* reg;

    gpio_msk_out_addr = regset->gpio_msk_out_addr;

    // create the mask in the upper 8 bits
    data = 0x0100 << pin;
    // set the lower bit to the desired value
    if (val) {
        data |= (1 << pin);
    } else {
        // don't need to do anything, since the lower bit is already 0
    }

    // write the data to the gpio_msk_out_addr
    if (GPIO_DBG) printf("writing 0x%04x to gpio_msk_out_addr 0x%08x\n", data, gpio_msk_out_addr);
    reg = (uint16_t *)(gpio_mapped) + (gpio_msk_out_addr - GPIO_BASE);
    *reg = data;

    return(0);
}

int
tx1_print_gpio_status_all(void) {
    // prints the contents of all the gpio registers for each controller
    int c, p;
    char portchar;
    uint32_t gpio_cnf_addr;
    uint32_t gpio_msk_cnf_addr;
    uint32_t gpio_oe_addr;
    uint32_t gpio_msk_oe_addr;
    uint32_t gpio_out_addr;
    uint32_t gpio_msk_out_addr;
    uint32_t gpio_in_addr;
    volatile uint16_t* reg;
    uint16_t data[7];

    portchar = 'A';
    for (c=1; c<9; c++) {
        printf("GPIO Controller %d\n", c);
        for (p = 0; p < 4; p++) {
            if (portchar <= 'Z') {
                printf("  port%c [%d]: ", portchar, p);
            } else {
                printf("  port%c%c [%d]: ", portchar - 26, portchar - 26, p);
            }
            portchar++;

            gpio_cnf_addr = GPIO_BASE_ADDR[c - 1] + GPIO_CNF_OFFSET[p];
            gpio_msk_cnf_addr = GPIO_BASE_ADDR[c - 1] + GPIO_MSK_CNF_OFFSET[p];
            gpio_oe_addr = GPIO_BASE_ADDR[c - 1] + GPIO_OE_OFFSET[p];
            gpio_msk_oe_addr = GPIO_BASE_ADDR[c - 1] + GPIO_MSK_OE_OFFSET[p];
            gpio_out_addr = GPIO_BASE_ADDR[c - 1] + GPIO_OUT_OFFSET[p];
            gpio_msk_out_addr = GPIO_BASE_ADDR[c - 1] + GPIO_MSK_OUT_OFFSET[p];
            gpio_in_addr = GPIO_BASE_ADDR[c - 1] + GPIO_IN_OFFSET[p];

            reg = (uint16_t *) (gpio_mapped) + (gpio_cnf_addr - GPIO_BASE);
            data[0] = *reg;
            reg = (uint16_t *) (gpio_mapped) + (gpio_msk_cnf_addr - GPIO_BASE);
            data[1] = *reg;
            reg = (uint16_t *) (gpio_mapped) + (gpio_oe_addr - GPIO_BASE);
            data[2] = *reg;
            reg = (uint16_t *) (gpio_mapped) + (gpio_msk_oe_addr - GPIO_BASE);
            data[3] = *reg;
            reg = (uint16_t *) (gpio_mapped) + (gpio_out_addr - GPIO_BASE);
            data[4] = *reg;
            reg = (uint16_t *) (gpio_mapped) + (gpio_msk_out_addr - GPIO_BASE);
            data[5] = *reg;
            reg = (uint16_t *) (gpio_mapped) + (gpio_in_addr - GPIO_BASE);
            data[6] = *reg;
            printf("    CNF=%04x, MSK_CNF=%04x, OE=%04x, MSK_OE=%04x, OUT=%04x, MSK_OUT=%04x, IN=%04x\n",
                   data[0], data[1], data[2], data[3], data[4], data[5], data[6]);
        }
    }
    return(0);
}

int
tx1_print_gpio_status(reg_t *regset) {
    // prints the contents of the registers for a particular port
    int c, p;
    uint32_t gpio_cnf_addr;
    uint32_t gpio_msk_cnf_addr;
    uint32_t gpio_oe_addr;
    uint32_t gpio_msk_oe_addr;
    uint32_t gpio_out_addr;
    uint32_t gpio_msk_out_addr;
    uint32_t gpio_in_addr;
    char portchar;
    volatile uint16_t* reg;
    uint16_t data[7];

    gpio_cnf_addr = regset->gpio_cnf_addr;
    gpio_msk_cnf_addr = regset->gpio_msk_cnf_addr;
    gpio_oe_addr = regset->gpio_oe_addr;
    gpio_msk_oe_addr = regset->gpio_msk_oe_addr;
    gpio_out_addr = regset->gpio_out_addr;
    gpio_msk_out_addr = regset->gpio_msk_out_addr;
    gpio_in_addr = regset->gpio_in_addr;
    portchar = regset->portchar;

    reg = (uint16_t *) (gpio_mapped) + (gpio_cnf_addr - GPIO_BASE);
    data[0] = *reg;
    reg = (uint16_t *) (gpio_mapped) + (gpio_msk_cnf_addr - GPIO_BASE);
    data[1] = *reg;
    reg = (uint16_t *) (gpio_mapped) + (gpio_oe_addr - GPIO_BASE);
    data[2] = *reg;
    reg = (uint16_t *) (gpio_mapped) + (gpio_msk_oe_addr - GPIO_BASE);
    data[3] = *reg;
    reg = (uint16_t *) (gpio_mapped) + (gpio_out_addr - GPIO_BASE);
    data[4] = *reg;
    reg = (uint16_t *) (gpio_mapped) + (gpio_msk_out_addr - GPIO_BASE);
    data[5] = *reg;
    reg = (uint16_t *) (gpio_mapped) + (gpio_in_addr - GPIO_BASE);
    data[6] = *reg;

    printf("port%c: CNF=%04x, MSK_CNF=%04x, OE=%04x, MSK_OE=%04x, OUT=%04x, MSK_OUT=%04x, IN=%04x\n",
           portchar, data[0], data[1], data[2], data[3], data[4], data[5], data[6]);

    return(0);
}
