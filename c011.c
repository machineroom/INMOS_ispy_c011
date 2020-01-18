#include <pigpio.h>
#include <time.h>
#include "pins.h"
#include "c011.h"
#include <stdio.h>
#include <stdlib.h>

#define TCSLCSH (60)
#define TCSHCSL (50)
#define TCSLDrV (50)

static void sleep_ns(int ns) {
    struct timespec s = {0,ns};
    int ret = nanosleep(&s,NULL);
    if (ret != 0) {
        printf ("nanosleep(%d) failed\n", ret);
    }
}

static void sleep_us(int us) {
    sleep_ns(us*1000);
}

static void sleep_ms(int ms) {
    sleep_us(ms*1000);
}

static void set_control_output_pins(void) {
    gpioSetMode(RS0, PI_OUTPUT);
    gpioSetMode(RS1, PI_OUTPUT);
    gpioSetMode(RESET, PI_OUTPUT);
    gpioSetMode(CS, PI_OUTPUT);
    gpioSetMode(RW, PI_OUTPUT);
}

static void set_data_output_pins(void) {
    gpioSetMode(D0, PI_OUTPUT);
    gpioSetMode(D1, PI_OUTPUT);
    gpioSetMode(D2, PI_OUTPUT);
    gpioSetMode(D3, PI_OUTPUT);
    gpioSetMode(D4, PI_OUTPUT);
    gpioSetMode(D5, PI_OUTPUT);
    gpioSetMode(D6, PI_OUTPUT);
    gpioSetMode(D7, PI_OUTPUT);
}

static void set_data_input_pins(void) {
    gpioSetMode(D0, PI_INPUT);
    gpioSetMode(D1, PI_INPUT);
    gpioSetMode(D2, PI_INPUT);
    gpioSetMode(D3, PI_INPUT);
    gpioSetMode(D4, PI_INPUT);
    gpioSetMode(D5, PI_INPUT);
    gpioSetMode(D6, PI_INPUT);
    gpioSetMode(D7, PI_INPUT);
}
        
void c011_init(void) {
    if (gpioInitialise() < 0) {
        printf ("gpioInitialise failed\n");
        exit(-1);
    }
    set_control_output_pins();
    gpioWrite(ANALYSE, LOW);
}

void c011_reset(void) {
    gpioWrite(ANALYSE, LOW);
    //TN29 states "Recommended pulse width is 5 ms, with a delay of 5 ms before sending anything down a link."
    gpioWrite(RESET, LOW);
    gpioWrite(RESET, HIGH);
    sleep_us (5*1000);
    gpioWrite(RESET, LOW);
    sleep_us (5*1000);
    //The whitecross HSL takes some time to cascade reset
    sleep_ms(600);
}

void c011_analyse(void) {
    gpioWrite(ANALYSE, LOW);
    sleep_us (5*1000);
    gpioWrite(ANALYSE, HIGH);
    sleep_us (5*1000);
    gpioWrite(RESET, HIGH);
    sleep_us (5*1000);
    gpioWrite(RESET, LOW);
    sleep_us (5*1000);
    gpioWrite(ANALYSE, LOW);
    sleep_us (5*1000);
}

void c011_enable_out_int(void) {
    /*set_data_output_pins();
    bcm2835_gpio_write_mask (1<<RS1 | 1<<RS0 | 0<<RW | 1<<CS,
                             1<<RS1 | 1<<RS0 | 1<<RW | 1<<CS);
    bcm2835_gpio_write_mask (1<<D1,
                             1<<D7 | 1<<D6 | 1<<D5 | 1<<D4 | 1<<D3 | 1<<D2 | 1<<D1 | 1<<D0);

    gpioWrite(CS, LOW);
    sleep_ns (TCSLCSH);
    gpioWrite(CS, HIGH);
    sleep_ns (TCSHCSL);
    */
}

void c011_enable_in_int(void) {
/*
    set_data_output_pins();
    bcm2835_gpio_write_mask (1<<RS1 | 0<<RS0 | 0<<RW | 1<<CS,
                             1<<RS1 | 1<<RS0 | 1<<RW | 1<<CS);
    bcm2835_gpio_write_mask (1<<D1,
                             1<<D7 | 1<<D6 | 1<<D5 | 1<<D4 | 1<<D3 | 1<<D2 | 1<<D1 | 1<<D0);
    gpioWrite(CS, LOW);
    sleep_ns (TCSLCSH);
    gpioWrite(CS, HIGH);
    sleep_ns (TCSHCSL);
    */
}

int c011_write_byte(uint8_t byte, uint32_t timeout) {
    //wait for output ready
    while ((c011_read_output_status() & 0x01) != 0x01 && timeout>0) {
        sleep_us(1000);
        timeout--;
    }
    if (timeout == 0) {
        return -1;
    }
    //RS1=0, RS0=1
    //RW=0
    //CS=1
    set_data_output_pins();
    gpioWrite_Bits_0_31_Clear (1<<RS1|1<<RW);
    gpioWrite_Bits_0_31_Set (1<<RS0|1<<CS);
    
    //D0-D7
    uint8_t d7,d6,d5,d4,d3,d2,d1,d0;
    d7=(byte&0x80) >> 7;
    d6=(byte&0x40) >> 6;
    d5=(byte&0x20) >> 5;
    d4=(byte&0x10) >> 4;
    d3=(byte&0x08) >> 3;
    d2=(byte&0x04) >> 2;
    d1=(byte&0x02) >> 1;
    d0=(byte&0x01) >> 0;
    gpioWrite_Bits_0_31_Clear (1<<D7 | 1<<D6 | 1<<D5 | 1<<D4 | 1<<D3 | 1<<D2 | 1<<D1 | 1<<D0);
    gpioWrite_Bits_0_31_Set (d7<<D7 | d6<<D6 | d5<<D5 | d4<<D4 | d3<<D3 | d2<<D2 | d1<<D1 | d0<<D0);
    //CS=0
    gpioWrite(CS, LOW);
    sleep_ns (TCSLCSH);
    //CS=1
    gpioWrite(CS, HIGH);
    sleep_ns (TCSHCSL);
    return 0;
}

static uint8_t read_c011(void) {
    uint8_t d7,d6,d5,d4,d3,d2,d1,d0,byte;
    set_data_input_pins();
    gpioWrite(CS, LOW);
    //must allow time for data valid after !CS
    sleep_ns (TCSLDrV);
    uint32_t gpio = gpioRead_Bits_0_31();
    d7=(gpio & 1<<D7)!=0;
    d6=(gpio & 1<<D6)!=0;
    d5=(gpio & 1<<D5)!=0;
    d4=(gpio & 1<<D4)!=0;
    d3=(gpio & 1<<D3)!=0;
    d2=(gpio & 1<<D2)!=0;
    d1=(gpio & 1<<D1)!=0;
    d0=(gpio & 1<<D0)!=0;
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
    gpioWrite(CS, HIGH);
    sleep_ns (TCSHCSL);
    return byte;
}

uint8_t c011_read_input_status(void) {
    uint8_t byte;
    gpioWrite_Bits_0_31_Clear (1<<RS0);
    gpioWrite_Bits_0_31_Set (1<<RS1|1<<CS|1<<RW);
    byte = read_c011();
    return byte;
}

uint8_t c011_read_output_status(void) {
    uint8_t byte;
    gpioWrite_Bits_0_31_Set (1<<RS1|1<<RS0|1<<CS|1<<RW);
    byte = read_c011();
    return byte;
}

int c011_read_byte(uint8_t *byte, uint32_t timeout) {
    while ((c011_read_input_status() & 0x01) == 0x00 && timeout>0) {
        sleep_us(1000);
        timeout--;
    }
    if (timeout == 0) {
        return -1;
    }
    gpioWrite_Bits_0_31_Clear (1<<RS0|1<<RS1);
    gpioWrite_Bits_0_31_Set (1<<CS|1<<RW);
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

