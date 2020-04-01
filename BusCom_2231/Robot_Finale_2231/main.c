/*
 * -----------------------------------------------------
 *  P R O G R A M M A T I O N   S P I   E S I G E L E C
 *
 * Lauchpad v1.5 support
 *
 * M S P 4 3 0 G 2 2 3 1   -   SPI/SLAVE 3 Wires
 *
 * (c)-Yann DUCHEMIN / ESIGELEC - r.III162018 for CCS
 * --------------------------------------------------------------
 * La carte Launchpad est raccordée en SPI via l'USI B0
 *      SCLK : P1.5 / SCLK
 *      SIMO : P1.7 / SDI
 *      MOSI : P1.6 / SDO
 *
 * A la reception du caractère 1 la LED Rouge P1.0 est allumée
 *
 * A la reception du caractère 0 la LED Rouge P1.0 est eteinte
 *
 */
#include <msp430.h>
#include <intrinsics.h>

#include "ADC.h"
#include "send.h"

volatile unsigned char RXDta;
unsigned int capt = 0;
unsigned int state = 0;

void init_Timer( void )
{
    TACTL &= ~MC_0; // arret du timer
    TACCR0 = 62500;
    TACTL = (0 | TASSEL_2 | MC_3 | ID_3 | TACLR); // select TimerA source SMCLK, set mode to up-counting
    TACCTL0 = 0 | OUTMOD_7 | CCIE; // select timer compare mode
}



/*
 * main.c
 */
void main( void )
{
    // Stop watchdog timer to prevent time out reset
    WDTCTL = WDTPW | WDTHOLD;

    if(CALBC1_1MHZ==0xFF || CALDCO_1MHZ==0xFF)
    {
        __bis_SR_register(LPM4_bits);
    }
    else
    {
        // Factory Set.
        DCOCTL = 0;
        BCSCTL1 = CALBC1_1MHZ;
        DCOCTL = (0 | CALDCO_1MHZ);
    }



    init_send();
    ADC_init();
    init_Timer();
    // Wait for the SPI clock to be idle (low).
    while ((P1IN & BIT5)) ;

    USICTL0 &= ~USISWRST;

    //__bis_SR_register(LPM4_bits | GIE); // general interrupts enable & Low Power Mode
    __enable_interrupt();
    while(1)
    {
        if(state == 1)
        {
            if(capt > 500){
                capt = 0;
                TACCR0 = 0;
                Send_char_SPI('x');//Valeur d'arret
                TACCR0 = 62500;
            }
        }
    }
}

// --------------------------- R O U T I N E S   D ' I N T E R R U P T I O N S

/* ************************************************************************* */
/* VECTEUR INTERRUPTION USI                                                  */
/* ************************************************************************* */
#pragma vector=USI_VECTOR
__interrupt void universal_serial_interface(void)
{
    while( !(USICTL1 & USIIFG) );   // waiting char by USI counter flag
    RXDta = USISRL;

    if (RXDta == 0x01) //if the input buffer is 0x31 (mainly to read the buffer)
    {
        state = 1;
        P1OUT |= BIT0; //turn on LED
        //USISRL = 0x00;
    }
    else if (RXDta == 0x00)
    {
        state = 0;
        P1OUT &= ~BIT0; //turn off LED
        //USISRL = 0x00;
    }

    USISRL = '\0';
    USICNT &= ~USI16B;  // re-load counter & ignore USISRH
    USICNT = 0x08;      // 8 bits count, that re-enable USI for next transfert
}
//------------------------------------------------------------------ End ISR




#pragma vector=TIMERA0_VECTOR //timer utilisé pour lire les données des capteurs
__interrupt void detect(void)
{
    ADC_Demarrer_conversion(0x03); // on lit l'infrarouge du centre
    capt = ADC_Lire_resultat();


    TACTL &= ~TAIFG; //RAZ TAIFG
}






