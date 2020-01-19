/*
Definition of C011 <> RPI GPIO pins
bcm2835 lib uses 40 pin J8 connector pin numbering
*/

#include <bcm2835.h>

#define D0 RPI_V2_GPIO_P1_03    //BLACK     (GPIO pin 2)
#define D1 RPI_V2_GPIO_P1_05    //BROWN     (GPIO pin 3)
#define D2 RPI_V2_GPIO_P1_07    //RED       (GPIO pin 4)
#define D3 RPI_V2_GPIO_P1_11    //ORANGE    (GPIO pin 17)
#define D4 RPI_V2_GPIO_P1_12    //YELLOW    (GPIO pin 18)
#define D5 RPI_V2_GPIO_P1_13    //GREEN     (GPIO pin 27)
#define D6 RPI_V2_GPIO_P1_15    //BLUE      (GPIO pin 22)
#define D7 RPI_V2_GPIO_P1_16    //VIOLET    (GPIO pin 23)

#define RS0 RPI_V2_GPIO_P1_18   //GREY      (GPIO pin 24)
#define RS1 RPI_V2_GPIO_P1_19   //WHITE     (GPIO pin 10)
#define RESET RPI_V2_GPIO_P1_21 //YELLOW    (GPIO pin 9)
#define CS RPI_V2_GPIO_P1_22    //GREEN     (GPIO pin 25)
#define RW RPI_V2_GPIO_P1_23    //BLUE      (GPIO pin 11)
#define ANALYSE RPI_V2_GPIO_P1_24 //VIOLET  (GPIO pin 8)



