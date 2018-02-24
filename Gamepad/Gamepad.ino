 /*
  * PIN used:
  * PF1 A4
  * PF4 A3
  * PF5 A2
  * PF6 A1
  * PF7 A0
  * 
  * PD0 3 (INT0)
  * PD1 2 (INT1)
  * PB4 8
  * PB5 9
  * PB6 10
  * PB7 11
  * 
  * PIN unused: (maybe use them as lights or buttons)
  * PF0 A5
  * PD4 4
  * PC6 5
  * PD7 6
  * PE6 7
  * PD6 12
  * PC7 13
  */

#include "Joystick2.h"

// set the step value of your encoder, mine has 320 steps per circle
#define TURNTABLE_CIRCLE_ACTUAL 320

void setup() {
    DDRF &= (~0xf3); DDRB &= (~0xf0);
    PORTF |= 0xf3; PORTB |= 0xf0;

    DDRD &= ~0x03; // pin 2 and 3, INT0 and INT1
    PORTD |= 0x03;
    
    EICRA = _BV(ISC00) | _BV(ISC10);
    EIMSK = _BV(INT0) | _BV(INT1);
    EIFR  = _BV(INTF0) | _BV(INTF1);

  // Initialize Joystick Library
  Joystick.begin();
  Joystick.buttons = 0;
}

uint8_t tt_state = 0;
int16_t tt_val = 0;
uint32_t keys = 0;

// interrupts
void update_encoder(void) {
  uint8_t tmpstate = 0;

  tmpstate = ((PIND & 0x03) << 2) | (tt_state & 3);
  tt_state = tmpstate >> 2;
  
  switch (tmpstate)
  {
    case 0: case 5: case 10: case 15: break;
    case 1: case 7: case 8: case 14: tt_val++; break;
    case 2: case 4: case 11: case 13: tt_val--; break;
    case 3: case 12: tt_val += 2; break;
    default: tt_val -= 2; break;
  }

  tt_val = (tt_val + TURNTABLE_CIRCLE_ACTUAL) % TURNTABLE_CIRCLE_ACTUAL;
  
  return;
}

ISR(INT0_vect) { update_encoder(); }
ISR(INT1_vect) { update_encoder(); }

void loop() {
  Joystick.xAxis = (uint8_t)(tt_val * 256 / TURNTABLE_CIRCLE_ACTUAL);
  /*
   * change the pins here, I plugged all pins reversed so the codes look like this
   * ex: keys = ((PINB & 0xf0) >> 4) | (PINF & 0xf0) | ((PINF & 0x02) << 8);
   * 
   * notice A4(PF1) is at button10 which should be E3 of official controller
   */
  keys = (((PINB & 0x80) >> 7) | ((PINB & 0x40) >> 5) | ((PINB & 0x20) >> 3) | ((PINB & 0x10) >> 1) | ((PINF & 0x80) >> 3) | ((PINF & 0x40) >> 1) | ((PINF & 0x20) << 1) | ((PINF & 0x10) << 3) | ((PINF & 0x02) << 8));
  Joystick.buttons = (~keys & 0x2ff);
  Joystick.sendState();
}
