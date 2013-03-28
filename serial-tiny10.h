/*
 *  ATTINY10 Software Serial Library!!!  :)
 *
 *  Currently hardcoded to 600 baud.
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include "delay_accurate.h"

#ifndef NEW_BYTE_HANDLER
    #define NEW_BYTE_HANDLER serial_send
#endif

static const uint8_t TOGGLE_DEBUG_LINE = 0;
static const uint8_t SERIAL_TX_PIN = 0;

static inline void serial_setup();
static inline void timer_setup();
static inline void int_enable();
static inline void int_disable();
static inline void timer_start();
static inline void timer_stop();
static inline uint8_t timer_running();
static inline void timer_toggle_running();
void serial_send(uint8_t data);
void serial_send_string(const char *string);

/*
Current bit index
*/
uint8_t bit_index = 0;

/*
Byte last received on the Rx line.  Will update bit-by-bit as receive sequence 
progresses, and is not cleared after being received.
*/
uint8_t outval = 0;

static inline void serial_setup(){
    /*
    Global serial setup function. Must be called before bytes can be received.
    */
    // Set Ouputs
    DDRB |= _BV(0) | _BV(1);
    PORTB = 0x01;

    timer_setup();
    int_enable();
    timer_start();
    //Global Interrupt Enable
    sei();
}

static inline void timer_setup(){
    /*
    General Setup for the timer module so that it can work for serial
    */
    TCCR0A = 0
        //| _BV(COM0A1) //
        //| _BV(COM0A0) //
        //| _BV(COM0B1) //
        //| _BV(COM0B0) //
        //| _BV(WGM01)  // waveform generation mode
        //| _BV(WGM00)  // waveform generation mode
        ;
    TCCR0B = 0
        //| _BV(ICNC0) //
        //| _BV(ICES0) //
        //| _BV(ICES0) //
        //| _BV(WGM03) // waveform generation mode
          | _BV(WGM02) // waveform generation mode
        ;
    TIMSK0 = 0 
          | _BV(OCIE0A) // output compare interrupt enable A
        //| _BV(OCIE0B) // output compare interrupt enable B
        //| _BV(TOIE0)  // timer overflow interrupt enable
        ;
}

/*
Timer clock source (prescaler) value
*/
static const uint8_t CSVAL = 0
      | _BV(CS00) //
    //| _BV(CS01) // 1/8
    //| _BV(CS02) //
    ;
static inline void timer_start(){
    /*
    Start the timer by setting the clock source as defined above
    */
    TCCR0B |= CSVAL;
}
static inline void timer_stop(){
    /*
    Stop the timer by clearing the clock source
    */
    TCCR0B &= ~CSVAL;
}
static inline uint8_t timer_running(){
    /*
    Returns whether the timer is running.  Assumes that if the timer is on,
    then it has the clock source defined above.
    */
    return TCCR0B & CSVAL;
}
static inline void timer_toggle_running(){
    /*
    Starts the timer if not running and vice versa.  Assumes that if the timer 
    is on, then it has the clock source defined above.
    */
    TCCR0B ^= CSVAL;
}

static inline void int_enable(){
    /*
    Enable interrupt INT0 (on pin 2) on falling edge.
    */
    EICRA = 0 // external interrupt control register A
        //| _BV(ISC00) // interrupt sense control
          | _BV(ISC01) // interrupt sense control
        ;
    EIMSK |= _BV(INT0); //external interrupt mask register
}
static inline void int_disable(){
    /*
    Disable interrupt INT0 (on pin 2).
    */
    EIMSK = 0;
}

void serial_send(uint8_t data){
    /*  
    Synchronously send one byte of serial data on the pin specified by 
    SERIAL_PIN. Data is sent at 600 baud
    */
    uint8_t mask = 1;
    //start bit
    PORTB &= ~_BV(SERIAL_TX_PIN);
    _delay_loop_3(416);
    do{
        if (data & mask){
            PORTB |= _BV(SERIAL_TX_PIN);
        }
        else{
            PORTB &= ~_BV(SERIAL_TX_PIN);
        }
        mask = mask << 1;
        _delay_loop_3(416);
    } while(mask);
    //stop bit
    PORTB |= _BV(SERIAL_TX_PIN);
    _delay_loop_3(416);
}
void serial_send_string(const char *string){
    /*
    Synchronously send a null-terminated string. Uses serial_send() above.
    */
    while(*string){
        serial_send((uint8_t)(*string));
        string++;
    }
}

ISR(TIM0_COMPA_vect) {
    /*
    Interrupt service routine called when TCNT0 == OCR0A.
    */
    outval |= ((PINB >> 2) & 0x01) << (bit_index++);
    if(TOGGLE_DEBUG_LINE){
        PORTB ^= _BV(1); //debug line
    }
    if(bit_index == 8){
        timer_stop();
        EIFR = 0x01; //for debouncing
        int_enable();
        //handle new byte received here!!!
        NEW_BYTE_HANDLER(outval);
    }
    OCR0A = 1667;
}

ISR(INT0_vect){
    /*
    Called on falling edge of INT0 (pin 2).
    */
    outval = 0;
    bit_index = 0;
    OCR0A = 2500;
    TCNT0 = 0;
    timer_start();
    if(TOGGLE_DEBUG_LINE){
        PORTB ^= _BV(1); //debug line
    }
    int_disable();
    EIFR = 0x01; //for debouncing
}
