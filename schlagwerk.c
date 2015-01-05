/* see copyright.txt for original copyright */

#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/power.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <string.h>
#include "Descriptors.h"

#include <LUFA/Drivers/Board/LEDs.h>
#include <LUFA/Drivers/USB/USB.h>
#include <LUFA/Platform/Platform.h>

USB_ClassInfo_MIDI_Device_t Keyboard_MIDI_Interface = {
  .Config = {
    .StreamingInterfaceNumber = INTERFACE_ID_AudioStream,
    .DataINEndpoint           = {
      .Address          = MIDI_STREAM_IN_EPADDR,
      .Size             = MIDI_STREAM_EPSIZE,
      .Banks            = 1,
    },
    .DataOUTEndpoint          = {
      .Address          = MIDI_STREAM_OUT_EPADDR,
      .Size             = MIDI_STREAM_EPSIZE,
      .Banks            = 1,
    }
  }
};

/** Configures the board hardware and chip peripherals. */
void SetupHardware(void)
{
  /* Disable watchdog if enabled by bootloader/fuses */
  MCUSR &= ~(1 << WDRF);
  wdt_disable();

  /* Disable clock division */
  clock_prescale_set(clock_div_1);

  USB_Init();

  // Timer 0 konfigurieren
  TCCR0A = (1<<WGM01); // CTC Modus
  TCCR0B |= (1<<CS01); // Prescaler 8
  // ((16000000/8)/100000) = 20 (100khz)
  OCR0A = 20-1;
 
  // Compare Interrupt erlauben
  TIMSK0 |= (1<<OCIE0A);
}

void EVENT_USB_Device_ConfigurationChanged(void)
{
  MIDI_Device_ConfigureEndpoints(&Keyboard_MIDI_Interface);
}

void EVENT_USB_Device_ControlRequest(void)
{
  MIDI_Device_ProcessControlRequest(&Keyboard_MIDI_Interface);
}

static uint8_t sample_buffer[128][4];
static uint8_t sample_pointer = 0;

ISR (TIMER0_COMPA_vect)
{
  PORTB = sample_buffer[sample_pointer][0];
  PORTC = sample_buffer[sample_pointer][1];
  PORTD = sample_buffer[sample_pointer][2];
  PORTF = sample_buffer[sample_pointer][3];
  sample_pointer++;
}

void
set_channel(uint8_t channel, uint8_t value)
{
  uint8_t byte_index = channel >> 3;
  uint8_t mask = 1 << (channel & 0x07);
  for (uint8_t i = 0; i < 128; i++) {
    if (i < value) {
      sample_buffer[i][byte_index] |= mask;
      continue;
    } else if (i == value) {
      mask = ~mask;
    }
    sample_buffer[i][byte_index] &= mask;
  }
}

int
main(void)
{
  SetupHardware();

  GlobalInterruptEnable();

  for (;;) {
    MIDI_EventPacket_t ReceivedMIDIEvent;
    while (MIDI_Device_ReceiveEventPacket(&Keyboard_MIDI_Interface, &ReceivedMIDIEvent)) {
      if (ReceivedMIDIEvent.Data2 < 32) {
        if (ReceivedMIDIEvent.Event == MIDI_EVENT(0, MIDI_COMMAND_NOTE_ON)) {
          set_channel(ReceivedMIDIEvent.Data2, ReceivedMIDIEvent.Data3);
        } else if (ReceivedMIDIEvent.Event == MIDI_EVENT(0, MIDI_COMMAND_NOTE_OFF)) {
          set_channel(ReceivedMIDIEvent.Data2, 0);
        }
      }
    }

    MIDI_Device_USBTask(&Keyboard_MIDI_Interface);
    USB_USBTask();
  }
}


