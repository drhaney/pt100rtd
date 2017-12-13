# PT100RTD

#### An Arduino Library for accurate Pt100 RTD ohms-to-Celsius conversion

## WHAT

It converts a Pt100 temperature sensor resistance into degrees Celsius 
using a lookup table taken from empirical data in the DIN 43760 / IEC 751 
document. This library's conversion accuracy is authoritative such that
other purely computational methods may be validated against it.


## WHY

Pt100 sensors can have uncalibrated accuracy which often exceeds that of the measurement
hardware and firmware. Now that these sensors are affordably manufactured at "1/10 DIN"
accuracy for the 0-100C range, the firmware should match them at least minimally. 


## WHY NOT

#### It's big.

Consuming ~3kB of Arduino program memory, this Pt100rtd library is larger
than any collection of computational methods that might be used instead. 
For any ordinary temperature between -60C and 650C, the venerable Callendar
-Van Dusen equation works well. Gas liquefaction enthusiasts, however,
have different requirements.

#### The measurement hardware is inadequate or middling.

If the hardware interface has insufficient resolution, an inaccurate reference
resistance (0.05% is only a start) or too high an excitation current through
the RTD, precise conversion can't correct inaccurate data acquisition.

A high excitation current causes RTD self-heating which militates against accurate
measurement. Self-heating can be corrected for only if the measurement conditions
are known beforehand . . . which they generally aren't.

Generally speaking, self-heating error is mitigated by liquid immersion thermometry
but reading an accurate air or gas temperature is more problematic.


## HOW

The Pt100 resistance lookup table uses unsigned 16-bit integers because:

DIN 43760 Pt100 resistances resolve at 0.01 ohms and are represented
in the lookup table as unsigned integer values of (ohms * 100) with
no loss of accuracy. Unsigned integers require 2 bytes vs. 4 bytes for
a floating point object. Integer arithmetic is also computationally cheaper
than software floating point operations, most significantly, numerical
comparison.

Even so, at 2100 bytes, the table being too large a global variable for 
SRAM, it resides in flash program memory with all the special handling 
that implies, specifically, PROGMEM data type(s) and use of 
pgm_read_word_near() to fetch them.  


## WTF

Several computational conversion methods are included for comparison: 
Callendar-Van Dusen (aka 'quadratic'), cubic, polynomial, and rational 
polynomial.  These functions are pedagogical and should be commented out 
eventually to save space.

If ported to a CPU with more SRAM and a floating point unit, (viz., ARM
Cortex M4 or better) these defs will certainly help: 
```C
	#define PROGMEM /**/
	#define pgm_read_word_near((x)) ((uint16_t)(*(x)))
```
	
It has been tested and and found suitable for the Adafruit Pt100 RTD
Breakout w/MAX31865 although any Arduino hardware+software mix that
produces a conformant Pt100 RTD output in ohms may use the library.  
    
   * http://www.adafruit.com/products/3328


Written by drhaney for his own selfish purposes under BSD 
license.  All text above must be included in any redistribution.
12/14/2017
