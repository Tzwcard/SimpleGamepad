/* 
 * PF0 A5
 * PF1 A4
 * PF4 A3
 * PF5 A2
 * PF6 A1
 * PF7 A0

 * PB4 8
 * PB5 9
 * PB6 10
 * PB7 11
 */

#include <Joystick.h>

// set the step value of your encoder
#define TURNTABLE_CIRCLE_ACTUAL 320

void setup() {
  DDRF &= (~0xf3); DDRB &= (~0xf0);
  PORTF |= 0xf3; PORTB |= 0xf0;

  DDRD &= ~0x03; // pin 2 and 3, INT0 and INT1
  PORTD |= 0x03;
    
  EICRA = _BV(ISC00) | _BV(ISC10);
  EIMSK = _BV(INT0) | _BV(INT1);
  EIFR  = _BV(INTF0) | _BV(INTF1);

  Joystick.begin(false);
}

uint8_t tt_state = 0;
int16_t tt_val = 0;
uint32_t keys = 0;

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
  Joystick.xAxis = (uint8_t)(double)(tt_val * 256 / 1.0 / TURNTABLE_CIRCLE_ACTUAL);
  keys = (((PINB & 0xf0) << 4) | (PINF & 0xf3));
  Joystick.buttons = (~keys & 0xff3) >> 2;
  Joystick.sendState();
}

