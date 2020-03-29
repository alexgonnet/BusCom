/* ---------------------------------------------------------------------------
 * Carte LAUCHPAD Rev.1.5 - MSP430G2553 - ESIGELEC
 * UltraSonic Sensor SRF05 (or aq.) + Carte d'extension
 * Vcc (3.3V)
 * ---------------------------------------------------------------------------
 * MSP430G2553
 * | 1 |Vcc Vss | 20 |
 * P1.0 | 2 | | 19 | P2.6-<LED]
 * P1.1 | 3 | | 18 | P2.7-<LED]
 * P1.2 | 4 | | 17 | Tst
 * P1.3 | 5 | | 16 | !RST
 * P1.4 | 6 | | 15 | P1.7
 * P1.5 | 7 | | 14 | P1.6-<Trigger]
 * P2.0 | 8 | | 13 | P2.5-<LED]
 * [Echo>-P2.1 | 9 | | 12 | P2.4-<LED]
 * P2.2 | 10 | | 11 | P2.3
 *
 * ---------------------------------------------------------------------------
 */
#include <msp430g2231.h>
#include <stdlib.h>
#include "send.h"

#define MEASUREMENT_PERIOD 3500
#define TRIGGER_PULSE 150

unsigned int t1 = 0, t2 = 0, distance = 0;
unsigned long delay_measurement = 0;

/* ----------------------------------------------------------------------------
 * Fonction d'initialisation de la carte TI LauchPAD
 * Entrees: -
 * Sorties: -
 */
void init_BOARD( void )
{
    // Stop watchdog timer to prevent time out reset
    WDTCTL = WDTPW | WDTHOLD;

    if( (CALBC1_1MHZ == 0xFF) || (CALDCO_1MHZ == 0xFF) )
    {
        __bis_SR_register(LPM4_bits); // #trap
    }
    else
    {
        // Factory Set.
        BCSCTL1 = (0 | CALBC1_1MHZ);
        DCOCTL = (0 | CALDCO_1MHZ);
    }

    //--------------- Secure mode
    P1SEL = 0x00; // GPIO
    P1DIR = 0x00; // IN


    P1SEL &= ~BIT0; // Port 1, ligne 0 et 6 en fonction primaire
        P1DIR |= BIT0; // P1.0 et P1.6 en sortie
        P1OUT &= BIT0; // P1.0 et P1.6 à 0

    // GPIO initialisation
    P1SEL &= ~(BIT6); // US-SRF Trigger Line out
    P1DIR |= (BIT6);
    P1OUT &= ~BIT6;

    P1SEL &= ~(BIT3); // LaunchPad Button
    P1DIR &= ~(BIT3);

    P1SEL |= BIT2; // US-SRF Echo line input capture
    P1DIR &= ~BIT2;

}

/* ----------------------------------------------------------------------------
 * Fonction d'initialisation du TIMER
 * Entree : -
 * Sorties: -
 */
void init_TIMER( void )
{
    TACTL &= ~MC_0; // arret du timer
    TACCTL0 |= CCIE;
    TACTL = TASSEL_2 | ID_0 | MC_1 | TAIE | TACLR;
    TACCR0 = TRIGGER_PULSE; // 0.000015 s

    TACTL = TASSEL_2 | ID_0 | MC_2 | TAIE | TACLR; // SMCLK; continuous mode, 0 -->FFFF
    //TACCR0 = MEASUREMENT_PERIOD - 1;
    TACCTL1 = CM_3 | CAP | CCIS_0 | SCS | CCIE | TAIE; // falling edge & raising edge,capture mode, capture/compare interrupt enable
    // P2.1 (echo) is connected to CCI1A (capture/compare input )
    TACCTL1 &= ~CCIFG;
}



void TXdata( unsigned char c )
{
    /*unsigned int i=0;
    unsigned char chaine[]=(unsigned char)distance;
    while(chaine[i] != '\0')
            {
                while (!(IFG2 & UCA0TXIFG));  // USCI_A0 TX buffer ready?
                UCA0TXBUF = chaine[i];              // TX -> RXed character
                //UCA0TXBUF = c;
                i++;
            }*/
    /*while (!(IFG2 & UCATXIFG)); { // USCI_A0 TX buffer ready?
            UCATXBUF = c;              // TX -> RXed character
    }*/
    Send_char_SPI(c);
}




/* ----------------------------------------------------------------------------
 * Fonction RADAR en utilisant les 4 LED de la carte d'extension
 * Entree : -
 * Sorties: -
 */
void radar( void )
{

    if (distance <= 30)
    {
        TXdata('a');
        P1OUT ^= BIT0;
    }
    else if ( (distance <= 50) && (distance >= 31) )
    {
        TXdata('b');
        P1OUT &= ~BIT0;
    }
    else if ( (distance <= 70) && (distance >= 51) )
    {
        TXdata('c');
        P1OUT ^= BIT0;
    }
    else if ( (distance <= 90) && (distance >= 71) )
    {
        TXdata('d');
        P1OUT &= ~BIT0;
    }
    else
    {
        TXdata('e');
        P1OUT ^= BIT0;
    }

}

/*
 * main.c
 *
 */
void main(void)
{
    init_BOARD();
    __no_operation();
    init_TIMER();
    init_send();
    P1IE |= BIT1;//selectionne le port de l'interruption
    P1IES |= BIT1;//selectionne le front
    P1IFG &= ~(BIT1);//descendre le flag
    __enable_interrupt();
    TXdata('>');
    TACCR1 = 1000;
    while(1)
    {
        TXdata('h');
        radar();
    }
}

// --------------------------- R O U T I N E S D ' I N T E R R U P T I O N S
/* ************************************************************************* */
/* VECTEUR INTERRUPTION TIMER0 */
/* ************************************************************************* */
#pragma vector = TIMERA0_VECTOR // Timer A Interrupt Service Routine
__interrupt void timer0_A0_isr(void)
{

    ( delay_measurement < MEASUREMENT_PERIOD ) ? ( delay_measurement++ ) : (delay_measurement = 0 );

    if( !delay_measurement )
    {
        TACCR0 = TRIGGER_PULSE; // 0.000015 s
        P1OUT |= BIT6; // Trigger ON
    }
    else
    {
        P1OUT &= ~BIT6; // trigger OFF
        TACCR0 = MEASUREMENT_PERIOD - 1;
    }

    TACTL &= ~TAIFG;
    TACCTL0 &= ~CCIFG; // unnecessary CCR0 has only one src
}

/* ************************************************************************* */
/* VECTEUR INTERRUPTION TIMER1 */
/* ************************************************************************* */
#pragma vector = TIMERA1_VECTOR
__interrupt void timer1_A1_isr(void)
{
    switch(_even_in_range(TAIV,TAIV_TAIFG))
    {
    case TAIV_NONE:
        break; // no more interrupts pending, exit ISR here.
    case TAIV_TACCR1: { // CCR1 interrupt - 0x0002
        if ( P1IN & BIT2 )
        { // detect rising edge
            t1 = TACCR1;
        }
        else { // and falling edge
            t2 = TACCR1;
            if (t2 > t1)
            {
                distance = (t2-t1)/59;
            }
            else
            {
                distance = 0;
            }
        }
        break;
    }
    case TAIV_6: // reserved
        distance = 0;
        break;
    case TAIV_8: // reserved
        distance = 0;
        break;
    case TAIV_TAIFG: // 0x000A
        distance = 0;
        break; // timer overflow
    default:
        break;
    }

    TACTL &= ~TAIFG;
    TACCTL1 &= ~CCIFG;
}

