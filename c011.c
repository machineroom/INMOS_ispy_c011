#include <bcm2835.h>
#include <time.h>
#include "pins.h"
#include "c011.h"
#include <stdio.h>
#include <stdlib.h>


#define TCSLCSH (6)
#define TCSHCSL (5)
#define TCSLDrV (5)

static uint8_t read_c011(void);

static uint32_t *gpio_base;

static void sleep_ns(int ns) {
    struct timespec s = {0,ns};
    int ret = nanosleep(&s,NULL);
    if (ret != 0) {
        printf ("nanosleep(%d) failed\n", ret);
    }
}

static void set_control_output_pins(void) {
    bcm2835_gpio_fsel(RS0, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(RS1, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(RESET, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(CS, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(RW, BCM2835_GPIO_FSEL_OUTP);
}

static void set_data_output_pins(void) {
    // SEL0 pins 0-9 output
    // pin rr  9  8  7  6  5  4  3  2  1  0
    //     00001001001001001001001001001001
    //        0   9   2   4   9   2   4   9
    bcm2835_peri_write_nb (gpio_base + BCM2835_GPFSEL0/4, 0x09249249UL);
    // SEL1 pins 10-19 output
    // pin rr 19 18 17 16 15 14 13 12 11 10
    //     00001001001001001001001001001001
    //        0   9   2   4   9   2   4   9
    bcm2835_peri_write_nb (gpio_base + BCM2835_GPFSEL1/4, 0x09249249UL);    
    // SEL2 pins 20-29 output       
    // pin rr 29 28 27 26 25 24 23 22 21 20
    //     00001001001001001001001001001001
    //        0   9   2   4   9   2   4   9
    bcm2835_peri_write_nb (gpio_base + BCM2835_GPFSEL2/4, 0x09249249UL);    
}

static void set_data_input_pins(void) {
    // SEL0 pins 0-7 input. 8-9 output
    // pin rr  9  8  7  6  5  4  3  2  1  0
    //     00001001000000000000000000000000
    //        0   9   0   0   0   0   0   0
    bcm2835_peri_write_nb (gpio_base + BCM2835_GPFSEL0/4, 0x09000000UL);
    // SEL1 pins 12-18 input. 10,11 output
    // pin rr 19 18 17 16 15 14 13 12 11 10
    //     00000000000000000000000000001001
    //        0   8   0   0   0   0   0   9
    bcm2835_peri_write_nb (gpio_base + BCM2835_GPFSEL1/4, 0x00000009UL);    
    // SEL2 pins 22,23,27 input. 24,25 output       
    // pin rr 29 28 27 26 25 24 23 22 21 20
    //     00000000000000001001000000000000
    //        0   0   0   0   9   0   0   0
    bcm2835_peri_write_nb (gpio_base + BCM2835_GPFSEL2/4, 0x00009000UL);    
}
        
void c011_init(void) {
    bcm2835_init();
    set_control_output_pins();
    bcm2835_gpio_write(ANALYSE, LOW);
    gpio_base = bcm2835_regbase(BCM2835_REGBASE_GPIO);
}

void c011_reset(void) {
    bcm2835_gpio_write(ANALYSE, LOW);
    //TN29 states "Recommended pulse width is 5 ms, with a delay of 5 ms before sending anything down a link."
    bcm2835_gpio_write(RESET, LOW);
    bcm2835_gpio_write(RESET, HIGH);
    bcm2835_delayMicroseconds (5*1000);
    bcm2835_gpio_write(RESET, LOW);
    bcm2835_delayMicroseconds (5*1000);
    //The whitecross HSL takes some time to cascade reset
    bcm2835_delay(600);
}

void c011_analyse(void) {
    bcm2835_gpio_write(ANALYSE, LOW);
    bcm2835_delayMicroseconds (5*1000);
    bcm2835_gpio_write(ANALYSE, HIGH);
    bcm2835_delayMicroseconds (5*1000);
    bcm2835_gpio_write(RESET, HIGH);
    bcm2835_delayMicroseconds (5*1000);
    bcm2835_gpio_write(RESET, LOW);
    bcm2835_delayMicroseconds (5*1000);
    bcm2835_gpio_write(ANALYSE, LOW);
    bcm2835_delayMicroseconds (5*1000);
}

void c011_enable_out_int(void) {
    abort ();   //TODO int not used
    /*
    set_data_output_pins();
    bcm2835_gpio_write_mask (1<<RS1 | 1<<RS0 | 0<<RW | 1<<CS,
                             1<<RS1 | 1<<RS0 | 1<<RW | 1<<CS);
    bcm2835_gpio_write_mask (1<<D1,
                             1<<D7 | 1<<D6 | 1<<D5 | 1<<D4 | 1<<D3 | 1<<D2 | 1<<D1 | 1<<D0);

    bcm2835_gpio_write(CS, LOW);
    sleep_ns (TCSLCSH);
    bcm2835_gpio_write(CS, HIGH);
    sleep_ns (TCSHCSL);
    */
}

void c011_enable_in_int(void) {
    abort ();
    /*    
    set_data_output_pins();
    bcm2835_gpio_write_mask (1<<RS1 | 0<<RS0 | 0<<RW | 1<<CS,
                             1<<RS1 | 1<<RS0 | 1<<RW | 1<<CS);
    bcm2835_gpio_write_mask (1<<D1,
                             1<<D7 | 1<<D6 | 1<<D5 | 1<<D4 | 1<<D3 | 1<<D2 | 1<<D1 | 1<<D0);
    bcm2835_gpio_write(CS, LOW);
    sleep_ns (TCSLCSH);
    bcm2835_gpio_write(CS, HIGH);
    sleep_ns (TCSHCSL);
    */
}

//timeout is ms
int c011_write_byte(uint8_t byte, uint32_t timeout) {
    timeout *= 1000;    //make us
    //wait for output ready
    //optimised read output status (don't set the control regs each time)
    bcm2835_gpio_write_mask (1<<RS1 | 1<<RS0 | 1<<RW | 1<<CS,
                             1<<RS1 | 1<<RS0 | 1<<RW | 1<<CS);
    uint8_t stat = read_c011();
    while ((stat & 0x01) != 0x01 && timeout>0) {
        bcm2835_delayMicroseconds(1);
        timeout--;
        stat = read_c011();
    }
    if (timeout == 0) {
        return -1;
    }
    //RS1=0, RS0=1
    //RW=0
    //CS=1
    set_data_output_pins();
    bcm2835_gpio_write_mask (0<<RS1 | 1<<RS0 | 0<<RW | 1<<CS,
                             1<<RS1 | 1<<RS0 | 1<<RW | 1<<CS);
    //D0-D7
    uint8_t d7,d6,d5,d4,d3,d2,d1,d0;
    d7=(byte&0x80) != 0;
    d6=(byte&0x40) != 0;
    d5=(byte&0x20) != 0;
    d4=(byte&0x10) != 0;
    d3=(byte&0x08) != 0;
    d2=(byte&0x04) != 0;
    d1=(byte&0x02) != 0;
    d0=(byte&0x01) != 0;
    bcm2835_gpio_write_mask (d7<<D7 | d6<<D6 | d5<<D5 | d4<<D4 | d3<<D3 | d2<<D2 | d1<<D1 | d0<<D0,
                             1<<D7 | 1<<D6 | 1<<D5 | 1<<D4 | 1<<D3 | 1<<D2 | 1<<D1 | 1<<D0);
    //CS=0
    bcm2835_gpio_write(CS, LOW);
    sleep_ns (TCSLCSH);
    //CS=1
    bcm2835_gpio_write(CS, HIGH);
    sleep_ns (TCSHCSL);
    return 0;
}

static uint8_t read_c011(void) {
    uint8_t d7,d6,d5,d4,d3,d2,d1,d0,byte;
    set_data_input_pins();
    bcm2835_gpio_write(CS, LOW);
    //must allow time for data valid after !CS
    sleep_ns (TCSLDrV);
    uint32_t reg = bcm2835_peri_read (gpio_base + BCM2835_GPLEV0/4);
    d7=(reg & 1<<D7) != 0;
    d6=(reg & 1<<D6) != 0;
    d5=(reg & 1<<D5) != 0;
    d4=(reg & 1<<D4) != 0;
    d3=(reg & 1<<D3) != 0;
    d2=(reg & 1<<D2) != 0;
    d1=(reg & 1<<D1) != 0;
    d0=(reg & 1<<D0) != 0;
    byte = d7;
    byte <<= 1;
    byte |= d6;
    byte <<= 1;
    byte |= d5;
    byte <<= 1;
    byte |= d4;
    byte <<= 1;
    byte |= d3;
    byte <<= 1;
    byte |= d2;
    byte <<= 1;
    byte |= d1;
    byte <<= 1;
    byte |= d0;
    bcm2835_gpio_write(CS, HIGH);
    sleep_ns (TCSHCSL);
    return byte;
}

uint8_t c011_read_input_status(void) {
    uint8_t byte;
    bcm2835_gpio_write_mask (1<<RS1 | 0<<RS0 | 1<<RW | 1<<CS,
                             1<<RS1 | 1<<RS0 | 1<<RW | 1<<CS);
    byte = read_c011();
    return byte;
}

uint8_t c011_read_output_status(void) {
    uint8_t byte;
    bcm2835_gpio_write_mask (1<<RS1 | 1<<RS0 | 1<<RW | 1<<CS,
                             1<<RS1 | 1<<RS0 | 1<<RW | 1<<CS);
    byte = read_c011();
    return byte;
}

//timeout is ms
int c011_read_byte(uint8_t *byte, uint32_t timeout) {
    timeout *= 1000;    //make us
    //optimised read input status (don't set the control regs each time)
    bcm2835_gpio_write_mask (1<<RS1 | 0<<RS0 | 1<<RW | 1<<CS,
                             1<<RS1 | 1<<RS0 | 1<<RW | 1<<CS);
    uint8_t stat = read_c011();
    while ((stat & 0x01) == 0x00 && timeout>0) {
        bcm2835_delayMicroseconds(1);
        timeout--;
        stat = read_c011();
    }
    if (timeout == 0) {
        return -1;
    }
    bcm2835_gpio_write_mask (0<<RS1 | 0<<RS0 | 1<<RW | 1<<CS,
                             1<<RS1 | 1<<RS0 | 1<<RW | 1<<CS);
    *byte = read_c011();
    return 0;
}

uint32_t c011_write_bytes (uint8_t *bytes, uint32_t num, uint32_t timeout) {
    uint32_t i;
    for (i=0; i < num; i++) {
        int ret = c011_write_byte(bytes[i], timeout);
        if (ret == -1) {
            break;
        }
    }
    return i;
}

uint32_t c011_read_bytes (uint8_t *bytes, uint32_t num, uint32_t timeout) {
    uint32_t i;
    for (i=0; i < num; i++) {
        int ret = c011_read_byte(&bytes[i], timeout);
        if (ret == -1) {
            break;
        }
    }
    return i;
}

