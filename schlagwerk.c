/* see copyright.txt for original copyright */

#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/power.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdbool.h>
#include <string.h>

/** Configures the board hardware and chip peripherals. */
void SetupHardware(void)
{
  /* Disable watchdog if enabled by bootloader/fuses */
  MCUSR &= ~(1 << WDRF);
  wdt_disable();

  /* Disable clock division */
  clock_prescale_set(clock_div_1);

  // Pins als Outputs definition
  DDRB = 0xff;
  DDRC = 0xff;
  DDRD = 0xff;
  DDRF = 0xff;
}

int
main(void)
{
  SetupHardware();

  for (;;) {
    PORTB = 0;
    PORTC = 0;
    PORTD = 0;
    PORTF = 0;
    _delay_ms(1000);
    PORTB = 255;
    PORTC = 255;
    PORTD = 255;
    PORTF = 255;
    _delay_ms(1000);
  }
}


