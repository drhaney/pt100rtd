#PT100RTD

Arduino Library for Pt100 RTD ohms-to-Celsius conversion

#WHAT

It converts a Pt100 temperature sensor resistance into degrees Celsius 
using a lookup table taken from empirical data in the DIN 43760 / IEC 751 
document.


#WHY

Its accuracy is authoritative such that other purely computational methods
may be validated against it.  There is nothing so powerful as a bad idea
whose time has come.


#WHY NOT

It's big.

Consuming ~3kB of Arduino program memory, this Pt100rtd library is larger
than any collection of computational methods that might be used instead. 
For any ordinary temperature between -60C and 650C, the venerable Callendar
-Van Dusen equation works well. Gas liquefaction enthusiasts, however, have
different requirements.


#HOW

The Pt100 resistance lookup table uses unsigned 16-bit integers because:

DIN 43760 Pt100 resistances resolve at 0.01 ohms and can be represented
in the lookup table as unsigned integer values of (ohms * 100) with
no loss of accuracy. Unsigned integers require 2 bytes vs. 4 bytes for
a floating point object. Integer arithmetic is also computationally cheaper
than software floating point operations, most significantly, numerical
comparison.

Even so, at 2100 bytes, the table being too large a global variable for 
SRAM, it resides in flash program memory with all the special handling 
that implies, specifically, PROGMEM data type(s) and use of 
pgm_read_word_near() to fetch them.  


#WTF

Several computational conversion methods are included for comparison: 
Callendar-Van Dusen (aka 'quadratic'), cubic, polynomial, and rational 
polynomial.  These functions are pedagogical and should be commented out 
eventually to save space.

If ported to a CPU with more SRAM and a floating point unit, (viz., ARM
Cortex M4 or better) these defs will certainly help: 

	#define PROGMEM /**/
	#define pgm_read_word_near((x)) (x)

It has been tested and and found suitable for the Adafruit Pt100 RTD
Breakout w/MAX31865 although any Arduino hardware+software mix that
produces a conformant Pt100 RTD output in ohms may use the library.  
    
   * http://www.adafruit.com/products/3328


Written by drhaney for his own selfish purposes under BSD 
license.  All text above must be included in any redistribution.
2/24/2017
