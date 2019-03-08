ATTINY10 Software UART
======================

An Interrupt-Driven UART Library for the Attiny10
-------------------------------------------------

The ATtiny10 lacks any hardware communication capabilities. This library solves
this problem by implementing a system to asynchronously receiving messages over
a serial datalink. The implementation contained within serial-tiny10.h uses 
interrupts and timers to sample the incoming signal. This allows you to do 
other things with the processor while simultaneously receiving data. I hope 
that this project will make it easier to build more complex and connected 
projects with the attiny10.

Note, the library depends on the delay loops contained in delay_accurate.h. 
It was necessary to create these new delay loops as the ones included in 
avr-libc are not compatible with the attiny10--they assume the availability of
certain instructions not implemented in the attiny10.

I'm interested in hearing from you if you find this library useful:  
http://www.kaylee.jp/contact.html
