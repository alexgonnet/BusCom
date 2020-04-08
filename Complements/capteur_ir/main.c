#include <msp430g2231.h>
#include "ADC.h"

int capt= 0;
int  infra_centre= 0;



void init_Timer( void )
{
TACTL &= ~MC_0; // arret du timer
TACCR0 = 62500;
TACTL = (0 | TASSEL_2 | MC_3 | ID_3 | TACLR); // select TimerA source SMCLK, set mode to up-counting
TACCTL0 = 0 | OUTMOD_7 | CCIE; // select timer compare mode
}



/**
 * main.c
 */
int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
    ADC_init();

    init_Timer();

    P1SEL &= ~(BIT0 | BIT6); // Port 1, ligne 0 et 6 en fonction primaire
    P1DIR |= (BIT0 | BIT6 ); // P1.0 et P1.6 en sortie
    P1OUT &= ~(BIT0 | BIT6); // P1.0 et P1.6 à 0

    P1DIR &= ~BIT3;             //P1.3 en entrée pour l'interrupteur
    P1SEL &= ~BIT3;             // en mode O/I

/*
    //timer pour asservissement
   // TACTL &= ~MC_0; // arret du timer

    TACTL=0|(TASSEL_2|ID_3);//sourceSMCLK,pasdepredivisionID_0
    TACTL|=MC_3;//comptageenmodeupdown

    //TACTL |= TACLR;

    TACTL|=TAIE;//autorisationinterruptionTAIE
    TACCR1=62500;//periode timer pour 1s

    TACCTL1 = 0 | OUTMOD_7 | CCIE; // select timer compare mode
*/
    __enable_interrupt();//Activation des interruptions



    while(1)
    {
    }

    return 0;
}





#pragma vector=TIMERA0_VECTOR //timer utilisé pour lire les données des capteurs
__interrupt void detect(void)
{
    //Equilibre();
    ADC_Demarrer_conversion(0x03); // on lit l'infrarouge du centre
    if(ADC_Lire_resultat() > 500) // si la valeur est dessus du seuil, on a un obstacle
    {
        P1OUT &=~BIT6; // Eteindre la Led du port 1 ligne 6
        if(infra_centre < 51) // on vérifie plusieurs fois la valeur pour éviter les erreurs
            infra_centre++;
        P1OUT ^= BIT0; // Faire clignoter la Led a chaque itération

    }
    else
    {
        P1OUT &=~BIT0; // Eteindre la Led du port 1 ligne 6
        if(infra_centre > -51)
            infra_centre--;
        P1OUT ^= BIT6; // Faire clignoter la Led a chaque itération
    }


    TACTL &= ~TAIFG; //RAZ TAIFG
}
