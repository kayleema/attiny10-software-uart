/*
Modified version of delay loops made to work with the attiny10.

_delay_loop_1 is simply copied from delay_basic.h in avr-libc, while
_delay_loop_3 is a modified version of _delay_loop_2 from delay_basic.h
*/

static inline void _delay_loop_1(uint8_t __count) __attribute__((always_inline));
static inline void _delay_loop_3(uint16_t __count) __attribute__((always_inline));

void _delay_loop_1(uint8_t __count){
    /*
    8-bit delay loop.  3-cycles per iteration.
     */
    __asm__ volatile (
        "1: dec %0 \n\t"
        "brne 1b"
        : "=r" (__count)
        : "0" (__count)
    );
}

void _delay_loop_3(uint16_t __count){
    /*
    16-bit delay loop.  4-cycles per iteration.
     */
    __asm__ volatile (
        "L_%=: subi %A0,1 \n\t" //subtract 1 from count low byte (1 cycle)
        "sbci %B0,0 \n\t" // subtract 1 (and cary) from high byte (1 cycle)
        "brne L_%= \n\t" // repeat from top if not zero (2 cycles)
        "nop \n\t"
        : "=r" (__count)
        : "0" (__count)
    );
}

