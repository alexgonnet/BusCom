#include <msp430g2231.h>
#include "ADC.h"


/* Timer initialization */
void init_Timer( void )
{
TACTL &= ~MC_0; /* Stop the timer */
TACCR0 = 62500;
TACTL = (0 | TASSEL_2 | MC_3 | ID_3 | TACLR); /* select TimerA source SMCLK, set mode to up-counting*/
TACCTL0 = 0 | OUTMOD_7 | CCIE; /* select timer compare mode*/
}



/**
 * main.c
 */
int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;	/* stop watchdog timer*/
    ADC_init();

    init_Timer();

    P1SEL &= ~(BIT0 | BIT6); /* Port 1, line 0 and 6 in primary function*/
    P1DIR |= (BIT0 | BIT6 ); /* P1.0 and P1.6 as output*/
    P1OUT &= ~(BIT0 | BIT6); /* P1.0 and P1.6 to 0*/

    P1DIR &= ~BIT3;             /*P1.3 input for interruption*/
    P1SEL &= ~BIT3;             /* O/I mode */

    __enable_interrupt();/*Enable interruptions*/



    while(1)
    {
    }

    return 0;
}




/* timer used to read the sensor value */
#pragma vector=TIMERA0_VECTOR
__interrupt void detect(void)
{
    ADC_Demarrer_conversion(0x03); /* Read the sensor value */
    if(ADC_Lire_resultat() > 500) /* If the value is above the limit value */
    {
        P1OUT &=~BIT6; /*Switch off the TED P1.6*/
        P1OUT ^= BIT0; /* Flash the LED on each iteration*/

    }
    else
    {
        P1OUT &=~BIT0; /* Switch off the TED P1.0*/
        P1OUT ^= BIT6; /* Flash the LED on each iteration*/
    }


    TACTL &= ~TAIFG; /*RAZ TAIFG*/
}
