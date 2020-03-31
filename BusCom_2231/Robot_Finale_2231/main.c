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
#include <stdio.h>

#include "ADC.h"
#include "send.h"

#define TRUE 1
#define FALSE 0

int capt= 0;
int  infra_centre= 0;
int danger = 0;

volatile unsigned char RXDta;


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

    //--------------- Secure mode
    P1SEL = 0x00;        // GPIO
    P1DIR = 0x00;         // IN


    // USI Config. for SPI 3 wires Slave Op.
    // P1SEL Ref. p41,42 SLAS694J used by USIPEx
    USICTL0 |= USISWRST;
    USICTL1 = 0;

    // 3 wire, mode Clk&Ph / 14.2.3 p400
    // SDI-SDO-SCLK - LSB First - Output Enable - Transp. Latch
    USICTL0 |= (USIPE7 | USIPE6 | USIPE5 | USILSB | USIOE | USIGE );
    // Slave Mode SLAU144J 14.2.3.2 p400
    USICTL0 &= ~(USIMST);
    USICTL1 |= USIIE;
    USICTL1 &= ~(USICKPH | USII2C);

    USICKCTL = 0;           // No Clk Src in slave mode
    USICKCTL &= ~(USICKPL | USISWCLK);  // Polarity - Input ClkLow

    USICNT = 0;
    USICNT &= ~(USI16B | USIIFGCC ); // Only lower 8 bits used 14.2.3.3 p 401 slau144j
    USISRL = 0x23;  // hash, just mean ready; USISRL Vs USIR by ~USI16B set to 0
    USICNT = 0x08;

    //init_send();
    ADC_init();

    init_Timer();

    P1SEL &= ~(BIT0); // Port 1, ligne 0 et 6 en fonction primaire
    P1DIR |= (BIT0); // P1.0 et P1.6 en sortie
    P1OUT &= ~(BIT0); // P1.0 et P1.6 à 0

    P1DIR &= ~BIT3;             //P1.3 en entrée pour l'interrupteur
    P1SEL &= ~BIT3;             // en mode O/I

    //__enable_interrupt();//Activation des interruptions
    __bis_SR_register(GIE); // general interrupts enable & Low Power Mode
    USICTL0 &= ~USISWRST;


        while(1)
        {
            if(danger == 1){
                danger = 0;
                //Send_char_SPI('b');
            }
        }

   // USICTL0 &= ~USISWRST;


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
    P1OUT |= BIT0;
    if (RXDta == 0x31) //if the input buffer is 0x31 (mainly to read the buffer)
    {
        P1OUT |= BIT0; //turn on LED
    }
    else if (RXDta == 0x30)
    {
        P1OUT &= ~BIT0; //turn off LED
    }

    //USISRL = RXDta;
    //USICNT &= ~USI16B;  // re-load counter & ignore USISRH
    //USICNT = 0x08;      // 8 bits count, that re-enable USI for next transfert
}
//------------------------------------------------------------------ End ISR



#pragma vector=TIMERA0_VECTOR //timer utilisé pour lire les données des capteurs
__interrupt void detect(void)
{
    //Equilibre();
    ADC_Demarrer_conversion(0x03); // on lit l'infrarouge du centre
    capt = ADC_Lire_resultat();
    if(capt > 500) // si la valeur est dessus du seuil, on a un obstacle
    {
        danger = 1;
        //P1OUT &=~BIT6; // Eteindre la Led du port 1 ligne 6
       // if(infra_centre < 51) // on vérifie plusieurs fois la valeur pour éviter les erreurs
         //   infra_centre++;
        //P1OUT ^= BIT0; // Faire clignoter la Led a chaque itération

    }
    else
    {
       // P1OUT &=~BIT0; // Eteindre la Led du port 1 ligne 6
       // if(infra_centre > -51)
         //   infra_centre--;

        //P1OUT ^= BIT6; // Faire clignoter la Led a chaque itération
    }



    TACTL &= ~TAIFG; //RAZ TAIFG
}
