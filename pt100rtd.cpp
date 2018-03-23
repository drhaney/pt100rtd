#include "pt100rtd.h"

/*******************************************************************
*	 pt100rtd contructor --
*
*******************************************************************/

pt100rtd::pt100rtd() { ; }


/**********************************************************************
** Function Name:	search_pt100_list
**
** Description:		binary search
**  	if match
**  	    return index of a match
**  	if no match
**  	    return index of the smallest table value > key
** 
**	usually requires the maximum of log2(1051) probes, ==10, 
**	when search key is not an exact match.
**
**	Note: search must not return index == 0.
**	Calling function must exclude boundary cases
**	where (ohmsX100 <= table[0]).
** 
** Parameters:
**		uint16_t ohmsX100
**
** Uses:
** Returns:	int index of nearest resistance value
** Creation: 1/26/2017 4:48a Daniel R. Haney
**********************************************************************/

int pt100rtd::search_pt100_list(uint16_t ohmsX100)
{
    int lower = 0 ;
    int upper = PT100_TABLE_MAXIDX ;
    int mid = (lower + upper) / 2 ;

    do
    {
	uint16_t pt100val = pgm_read_word_near(&Pt100_table[mid]) ;
		
	if (pt100val == ohmsX100)
	{
	    break;
	}
	else if (pt100val < ohmsX100)
	{
	    lower = mid + 1 ;
	}
	else
	{
	    upper = mid ;
	}
		
	mid = (lower + upper) / 2 ;
		
    } while (lower < upper)	;
	// falls through on last mismatch

    return(mid);
}

/**********************************************************************
** Function Name:	ohmsX100_to_celsius
**
** Description:
** 	Look up (unsigned short int)(Pt100 resistance * 100) in table.
**  	Interpolate temperature for intermediate resistances.
** 
**  	Calling function must exclude boundary cases where
**  	ohmsX100 <= table[0] && ohmsX100 >= table[MAX]
** 
** Parameters:
**	uint16_t Rrtd = 100 * (Pt100 RTD resistance in ohms)
**
** Uses:	Pt100_table
** Returns:	float temperature celsius
**
** Creation: 1/26/2017 10:41a Daniel R. Haney
**********************************************************************/
float pt100rtd::ohmsX100_to_celsius (uint16_t ohmsX100)
{
    uint16_t R_upper, R_lower ;
    int hundredths = 0 ; 		// STFU flag for avr-gcc
    int iTemp = 0 ;
    float celsius ;

    int index = search_pt100_list(ohmsX100) ;
	
    // The minimum integral temperature
    iTemp = index - 1 + CELSIUS_MIN ;
	
    // fetch floor() and ceiling() resistances since
    // key = intermediate value is the most likely case.

    // ACHTUNG!  (index == 0) is forbidden!
    R_lower = pgm_read_word_near(&Pt100_table[index - 1]) ;
    R_upper = pgm_read_word_near(&Pt100_table[index]) ;

    // if key == table entry, temp is an integer degree
    if (ohmsX100 == R_upper)
    {
	iTemp++ ;
	hundredths = 0 ;
    }
    // an intermediate resistance is the common case
    else if (ohmsX100 < R_upper)
    {
	hundredths = ((100 * (ohmsX100 - R_lower)) / (R_upper - R_lower)) ;
    }
    // two unlikely cases are included for disaster recovery
    else if (ohmsX100 > R_upper) /*NOTREACHED*/  /*...unless list search was dain bramaged */
    {
	iTemp++ ;
	// risks index+1 out of range
	uint16_t Rnext = pgm_read_word_near(&Pt100_table[index + 1]) ;
	hundredths = (100 * (ohmsX100 - R_upper)) / (Rnext - R_upper) ;
    }
    else	/*NOTREACHED*/  /*...except in cases of excessive tweakage at 2:30am */
    {
	hundredths = ((100 * (ohmsX100 - R_lower)) / (R_upper - R_lower)) ;
    }

    celsius  = (float)iTemp + (float)hundredths / 100.0 ;

    return(celsius );
}


/**********************************************************************
** Function Name:	celsius (uint16_t)
** Function Name:	celsius (float)
**
** Description:
** 		return celsius temperature for a given Pt100 RTD resistance
**
**		This wrapper function excludes boundary cases where
**  		ohmsX100 <= table[0] && ohmsX100 >= table[MAX]
** 
** Creation: 2/18/2017 2:29p Daniel R. Haney
**********************************************************************/

// Uses minimally-processed ADC binary output,
// an unsigned 16 bit integer == (ohms * 100).

float pt100rtd::celsius (uint16_t ohmsX100)
{
    // clip underflow	
    if (ohmsX100 <= pgm_read_word_near(&Pt100_table[0]))
    {
	// return min boundary temperature
	return((float) CELSIUS_MIN);
    }
    // clip overflow
    else if (ohmsX100 >= pgm_read_word_near(&Pt100_table[PT100_TABLE_MAXIDX]))
    {
	// return max boundary temperature
	return((float) CELSIUS_MAX);
    }
    else
    {
	return(pt100rtd::ohmsX100_to_celsius(ohmsX100)) ;
    }
}


// Uses a floating point resistance value.

float pt100rtd::celsius (float rtd_ohms)
{
    // convert to unsigned short
    uint16_t ohmsX100 = (uint16_t) floor(rtd_ohms * 100.0) ;
    return pt100rtd::celsius(ohmsX100) ;
}



/**********************************************************************
** Function Name:   celsius_to_Pt100ohms
**
** Description:	    Return a Pt100 resistance corresponding to a temperature.
** 		    Seemed like a handy thing to have.
**
** Parameters:      float celsius
**
** Uses:	    Pt100_table[], entries are uint16_t (ohms * 100)
** Returns:	    	float
** Creation: 2/24/2017 10:42a Daniel R. Haney
**********************************************************************/

float pt100rtd::celsius_to_Pt100ohms (float celsius)
{
    float Pt100_ohms, T_delta ;
    uint16_t R_lower, R_delta, R_fraction ;
    int upper, lower ;

    if (celsius < (float)CELSIUS_MIN)
    {
	R_lower = Pt100_table[0] ;
	R_fraction = 0 ;
    }
    else if (celsius > (float)CELSIUS_MAX)
    {
	R_lower = Pt100_table[PT100_TABLE_MAXIDX] ;
	R_fraction = 0 ;
    }
    else
    {
	lower = ((int) floor(celsius)) - CELSIUS_MIN ;
	upper = ((int) ceil(celsius)) - CELSIUS_MIN ; 

	R_lower = Pt100_table[lower] ;
	R_delta = Pt100_table[upper] - R_lower ;

	// fractional (non-integer) temperature
	T_delta = celsius - floor(celsius) ;

	// R_fraction is ohms * 100 (integer)
	// 0.5 is for round up before floor() truncation
	R_fraction = (uint16_t) floor(0.5 + (T_delta * (float)(R_delta))) ;
    }

    Pt100_ohms = ((float) (R_lower + R_fraction)) / 100.0 ;

    return(Pt100_ohms) ;
}


// inverse callendar van dusen formula.
// accurate from -60C up to 850 C.

#define PT100_NOMINAL 100.0
#define iCVD_A 3.9083e-3	// ganked from Adafruit_Max318656 library .h
#define iCVD_B -5.775e-7	// ditto.

float pt100rtd::celsius_cvd(float R_ohms)
{
    float Z1, Z2, Z3, Z4, temp;

    //Serial.print("Resistance: "); Serial.println(Rt, 8);

    Z1 = -iCVD_A;
    Z2 = iCVD_A * iCVD_A - (4 * iCVD_B);
    Z3 = (4 * iCVD_B) / PT100_NOMINAL;
    Z4 = 2 * iCVD_B;

    temp = Z2 + (Z3 * R_ohms);
    temp = (sqrt(temp) + Z1) / Z4;

    return (temp );
}


// cubic approximation
float pt100rtd::celsius_cubic(float R_ohms)
{
    float T = -247.29 + R_ohms * ( 2.3992 + R_ohms * (0.00063962 + 1.0241E-6 * R_ohms)) ;
    return(T );
}


// R2T polynomial from Analog Devices AN709 app note.
// implementation ganked from Adafruit MAX31865 library.
// Use for accurate temperatures -60C and below.
// Warning! Exceeds Class B tolerance spec above +164C

float pt100rtd::celsius_polynomial (float R_ohms)
{
    float rpoly, temp ;
    rpoly = R_ohms ;

    temp = -242.02 ;
    temp += 2.2228 * rpoly ;
    rpoly *= R_ohms ;			// square ;
    temp += 2.5859e-3 * rpoly ;
    rpoly *= R_ohms ;			// ^3 ;
    temp -= 4.8260e-6 * rpoly ;
    rpoly *= R_ohms ;			// ^4 ;
    temp -= 2.8183e-8 * rpoly ;
    rpoly *= R_ohms ;			// ^5 ;
    temp += 1.5243e-10 * rpoly ;

    return(temp);
}

// Rational polynomial fraction approximation taken from
// Mosaic Industries.com page on "RTD calibration."
// Accurate, probably beyond the ITS-90 spec
float pt100rtd::celsius_rationalpolynomial (float R_ohms)
{
    float num, denom, T ;

    float c0= -245.19 ;
    float c1 = 2.5293 ;
    float c2 = -0.066046 ;
    float c3 = 4.0422E-3 ;
    float c4 = -2.0697E-6 ;
    float c5 = -0.025422 ;
    float c6 = 1.6883E-3 ;
    float c7 = -1.3601E-6 ;

    num = R_ohms * (c1 + R_ohms * (c2 + R_ohms * (c3 + R_ohms * c4))) ;
    denom = 1.0 + R_ohms * (c5 + R_ohms * (c6 + R_ohms * c7)) ;
    T = c0 + (num / denom) ;

    return(T );
}

/*END*/
