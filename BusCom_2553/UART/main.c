//------------------------------------------------------------------------------
//
//
//
//------------------------------------------------------------------------------
#include <msp430g2553.h>
#include <string.h>
#include <stdlib.h>

typedef enum
{
    E_ACTIVE = 0x01,
    E_DESACTIVE = 0x00
} TE_State;

typedef struct
{
    TE_State etat;
    int valeur;
}S_Periph;


S_Periph servomoteur;
S_Periph infrarouge;
S_Periph ultrason;


void InitUART(void)
{
    P1SEL |= (BIT1 | BIT2);                     // P1.1 = RXD, P1.2=TXD
    P1SEL2 |= (BIT1 | BIT2);                    // P1.1 = RXD, P1.2=TXD
    UCA0CTL1 = UCSWRST;                         // SOFTWARE RESET
    UCA0CTL1 |= UCSSEL_3;                       // SMCLK (2 - 3)

    UCA0CTL0 &= ~(UCPEN | UCMSB | UCDORM);
    UCA0CTL0 &= ~(UC7BIT | UCSPB | UCMODE_3 | UCSYNC); // dta:8 stop:1 usci_mode3uartmode
    UCA0CTL1 &= ~UCSWRST;                   // **Initialize USCI state machine**

    UCA0BR0 = 104;                              // 1MHz, OSC16, 9600 (8Mhz : 52) : 8/115k
    UCA0BR1 = 0;                                // 1MHz, OSC16, 9600
    UCA0MCTL = 10;

    /* Enable USCI_A0 RX interrupt */
    IE2 |= UCA0RXIE;
}

void TXdata( unsigned char c )
{

    unsigned int i = 0;
    char valeur[50];
    unsigned char stringPrint[150];
    switch(c)
    {
        case 'a':
            P1OUT |= (BIT0|BIT6); // debug instruction
            infrarouge.etat = E_ACTIVE;
            ultrason.etat = E_ACTIVE;
            servomoteur.etat = E_ACTIVE;
            strcpy(stringPrint,"IR active\tUS active\tServomoteur active\n\r");
            while(stringPrint[i] != '\0')
            {
                while (!(IFG2 & UCA0TXIFG));  // USCI_A0 TX buffer ready?
                UCA0TXBUF = stringPrint[i];              // TX -> RXed character
                i++;
            }
            break;

        case 'd':
            strcpy(stringPrint,"Valeurs de debug:\n\r\tInfrarouge:  Etat: ");
            if(infrarouge.etat == E_ACTIVE)
            {
                strcat(stringPrint,"active\t Valeur: ");
                sprintf(valeur, "%d", infrarouge.valeur);
                strcat(stringPrint, valeur);
                strcat(stringPrint, "\n\r");
            }
            else
            {
                strcat(stringPrint,"desactive\t Valeur: 0\n\r");
            }
            while(stringPrint[i] != '\0')
            {
                while (!(IFG2 & UCA0TXIFG));  // USCI_A0 TX buffer ready?
                UCA0TXBUF = stringPrint[i];              // TX -> RXed character
                i++;
            }
            i = 0;
            strcpy(stringPrint,"\tUltrason:    Etat: ");
            if(ultrason.etat == E_ACTIVE)
            {
                strcat(stringPrint,"active\t Valeur: ");
                sprintf(valeur, "%d", ultrason.valeur);
                strcat(stringPrint, valeur);
                strcat(stringPrint, "\n\r");
            }
            else
            {
                strcat(stringPrint,"desactive\t Valeur: 0\n\r");
            }
            while(stringPrint[i] != '\0')
            {
                while (!(IFG2 & UCA0TXIFG));  // USCI_A0 TX buffer ready?
                UCA0TXBUF = stringPrint[i];              // TX -> RXed character
                i++;
            }
            i = 0;
            strcpy(stringPrint,"\tServomoteur: Etat: ");
            if(servomoteur.etat == E_ACTIVE)
            {
                strcat(stringPrint,"active\t Valeur: ");
                sprintf(valeur, "%d", servomoteur.valeur);
                strcat(stringPrint, valeur);
                strcat(stringPrint, "\n\r");
            }
            else
            {
                strcat(stringPrint,"desactive\t Valeur: 0\n\r");
            }

            while(stringPrint[i] != '\0')
            {
                while (!(IFG2 & UCA0TXIFG));  // USCI_A0 TX buffer ready?
                UCA0TXBUF = stringPrint[i];              // TX -> RXed character
                i++;
            }
            break;

        case 'h':
            strcpy(stringPrint,"aide:\n\r\ta: allumer les capteurs et le servomoteur\n\r\ti: allumer/eteindre le capteur infrarouge\n\r");
            while(stringPrint[i] != '\0')
            {
                while (!(IFG2 & UCA0TXIFG));  // USCI_A0 TX buffer ready?
                UCA0TXBUF = stringPrint[i];              // TX -> RXed character
                i++;
            }
            i = 0;
            strcpy(stringPrint,"\ts: allumer/eteindre le servomoteur\n\r\tx: eteindre les capteurs et le servomoteur\n\r\td: afficher les valeurs de debug\n\r\th: afficher l'aide\n\r");
            while(stringPrint[i] != '\0')
            {
                while (!(IFG2 & UCA0TXIFG));  // USCI_A0 TX buffer ready?
                UCA0TXBUF = stringPrint[i];              // TX -> RXed character
                i++;
            }
            break;

        case 'i':
            strcpy(stringPrint,"IR ");
            if(infrarouge.etat == E_DESACTIVE)
            {
                strcat(stringPrint,"active\n\r");
                infrarouge.etat = E_ACTIVE;
                P1OUT |= BIT0; // debug instruction
            }
            else
            {
                strcat(stringPrint,"desactive\n\r");
                infrarouge.etat = E_DESACTIVE;
                P1OUT &= ~BIT0; // debug instruction
            }
            while(stringPrint[i] != '\0')
            {
                while (!(IFG2 & UCA0TXIFG));  // USCI_A0 TX buffer ready?
                UCA0TXBUF = stringPrint[i];              // TX -> RXed character
                i++;
            }
            break;

        case 'u':
            strcpy(stringPrint,"US ");
            if(ultrason.etat == E_DESACTIVE)
            {
                strcat(stringPrint,"active\n\r");
                ultrason.etat = E_ACTIVE;
                P1OUT |= BIT6; // debug instruction
            }
            else
            {
               strcat(stringPrint,"desactive\n\r");
               ultrason.etat = E_DESACTIVE;
               P1OUT &= ~BIT6; // debug instruction
            }
            while(stringPrint[i] != '\0')
            {
                while (!(IFG2 & UCA0TXIFG));  // USCI_A0 TX buffer ready?
                UCA0TXBUF = stringPrint[i];              // TX -> RXed character
                i++;
            }
            break;

        case 's':
            strcpy(stringPrint,"Servomoteur ");
            if(servomoteur.etat == E_DESACTIVE)
            {
                strcat(stringPrint,"active\n\r");
                servomoteur.etat = E_ACTIVE;
            }
            else
            {
                strcat(stringPrint,"desactive\n\r");
                servomoteur.etat = E_DESACTIVE;
            }
            P1OUT ^= (BIT0|BIT6); // debug instruction
            __delay_cycles(1000); // debug instruction
            P1OUT ^= (BIT0|BIT6); // debug instruction
            while(stringPrint[i] != '\0')
            {
                while (!(IFG2 & UCA0TXIFG));  // USCI_A0 TX buffer ready?
                UCA0TXBUF = stringPrint[i];              // TX -> RXed character
                i++;
            }
            break;

        case 'x':
            P1OUT &= ~(BIT0|BIT6); // debug instruction
            infrarouge.etat = E_DESACTIVE;
            ultrason.etat = E_DESACTIVE;
            servomoteur.etat = E_DESACTIVE;
            strcpy(stringPrint,"IR desactive\tUS desactive\tServomoteur desactive\n\r");
            while(stringPrint[i] != '\0')
            {
                while (!(IFG2 & UCA0TXIFG));  // USCI_A0 TX buffer ready?
                UCA0TXBUF = stringPrint[i];              // TX -> RXed character
                i++;
            }
            break;

        default:
            strcpy(stringPrint,"commande non reconnue, veuillez entrer une nouvelle commande ou consulter l'aide en appuyant sur 'h'\n\r");
            while(stringPrint[i] != '\0')
            {
                while (!(IFG2 & UCA0TXIFG));  // USCI_A0 TX buffer ready?
                UCA0TXBUF = stringPrint[i];              // TX -> RXed character
                i++;
            }
            break;              // TX -> RXed character
    }
}

void main(void)
{
    P1DIR |= (BIT0|BIT6);
    P1SEL &= ~(BIT0|BIT6);
    P1SEL2 &= ~(BIT0|BIT6);
    P1OUT &= ~(BIT0|BIT6);
    // Stop watchdog timer to prevent time out reset
    WDTCTL = WDTPW | WDTHOLD;

    infrarouge.etat = E_DESACTIVE;
    infrarouge.valeur = 0;
    ultrason.etat = E_DESACTIVE;
    ultrason.valeur = 0;
    servomoteur.etat = E_DESACTIVE;
    servomoteur.valeur = 0;

    if(CALBC1_1MHZ==0xFF || CALDCO_1MHZ==0xFF)
    {
        __bis_SR_register(LPM4_bits); // Low Power Mode #trap on Error
    }
    else
    {
        // Factory parameters
        BCSCTL1 = CALBC1_1MHZ;
        DCOCTL = CALDCO_1MHZ;
    }

    InitUART();

    __bis_SR_register(GIE); // interrupts enabled

    TXdata('>');
    while(1);
}

// Echo back RXed character, confirm TX buffer is ready first
#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
{
    unsigned char c;

    c = UCA0RXBUF;
    TXdata(c);
}

