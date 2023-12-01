#ifndef GPIO_TX1_GPIO_H
#define GPIO_TX1_GPIO_H


//
// gpio.h
// rev 1 - shabaz - Nov 2023
//
#include <stdint.h>


#define GPIO_1 0x6000d000
#define GPIO_2 0x6000d100
#define GPIO_3 0x6000d200
#define GPIO_4 0x6000d300
#define GPIO_5 0x6000d400
#define GPIO_6 0x6000d500
#define GPIO_7 0x6000d600
#define GPIO_8 0x6000d700
#define GPIO_MAP_SIZE (0x100 * 8)
#define GPIO_BASE GPIO_1

#define GPIO_CNF_0 0x00
#define GPIO_CNF_1 0x04
#define GPIO_CNF_2 0x08
#define GPIO_CNF_3 0x0c
#define GPIO_MSK_CNF_0 0x80
#define GPIO_MSK_CNF_1 0x84
#define GPIO_MSK_CNF_2 0x88
#define GPIO_MSK_CNF_3 0x8c
#define GPIO_OE_0 0x10
#define GPIO_OE_1 0x14
#define GPIO_OE_2 0x18
#define GPIO_OE_3 0x1c
#define GPIO_MSK_OE_0 0x90
#define GPIO_MSK_OE_1 0x94
#define GPIO_MSK_OE_2 0x98
#define GPIO_MSK_OE_3 0x9c
#define GPIO_OUT_0 0x20
#define GPIO_OUT_1 0x24
#define GPIO_OUT_2 0x28
#define GPIO_OUT_3 0x2c
#define GPIO_MSK_OUT_0 0xa0
#define GPIO_MSK_OUT_1 0xa4
#define GPIO_MSK_OUT_2 0xa8
#define GPIO_MSK_OUT_3 0xac
#define GPIO_IN_0 0x30
#define GPIO_IN_1 0x34
#define GPIO_IN_2 0x38
#define GPIO_IN_3 0x3c

// there are 8 controllers, GPIO_1 to GPIO_8. Each controller has 4 ports,
// called A-D, E-H, I-L and so on.
// Ports have 8 pins each. For example GPIO3_PC_03 means port C, pin 3.
// to translate from a GPIO name, e.g. GPIO3_PC.03
// to the registers for it, this is the procedure.
// (a) Convert the letter to a number, e.g. A -> 0, B -> 1, C -> 2
// (b) divide that by 4, (i.e. n/4) and that number will indicate the controller,
//     where 0 is GPIO_1, 1 is GPIO_2 and so on.
// (c) the remainder (i.e. n%4) will indicate the port, where 0 is port A or port E or port I
//     and so on.


typedef struct reg_s {
    uint32_t gpio_cnf_addr;
    uint32_t gpio_msk_cnf_addr;
    uint32_t gpio_oe_addr;
    uint32_t gpio_msk_oe_addr;
    uint32_t gpio_out_addr;
    uint32_t gpio_msk_out_addr;
    uint32_t gpio_in_addr;
    uint8_t controller; // 1-8
    uint8_t port; // 0-3
    char pin; // 0-7
    char portchar; // A-Z
} reg_t;

int gpio_init(void);
int tx1_print_gpio_status(void);
int tx1_print_gpio_status(reg_t *regset);
int tx1_gpio_name_decode(const char *gpio_name, reg_t *regset);
int tx1_gpio_set_output(reg_t *regset, char pin);
int tx1_gpio_set_value(reg_t *regset, char pin, char val);
int gpio_close(void);



#endif //GPIO_TX1_GPIO_H
