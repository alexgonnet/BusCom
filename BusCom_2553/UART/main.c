//------------------------------------------------------------------------------
//
//
//
//------------------------------------------------------------------------------
#include <msp430g2553.h>
#include <string.h>
#include <stdlib.h>
#include "Dplc.h""

#define RECULER 1
#define AVANCER 2
#define GAUCHE 3
#define DROITE 4


#define LF      0x0A            // line feed or \n
#define CR      0x0D            // carriage return or \r
#define BSPC    0x08            // back space
#define DEL     0x7F            // SUPRESS
#define ESC     0x1B            // escape

#define _CS         BIT4            // chip select for SPI Master->Slave ONLY on 4 wires Mode
#define SCK         BIT5            // Serial Clock
#define DATA_OUT    BIT6            // DATA out
#define DATA_IN     BIT7            // DATA in

unsigned char cmd[10];      // tableau de caracteres lie a la commande user

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

void init_USCI( void );
void Send_char_SPI2( unsigned char );


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


/* ----------------------------------------------------------------------------
 * Fonction d'initialisation de l'USCI POUR SPI SUR UCB0
 * Entree : -
 * Sorties: -
 */
void init_USCI( void )
{
    // Waste Time, waiting Slave SYNC
    __delay_cycles(250);

    // SOFTWARE RESET - mode configuration
    UCB0CTL0 = 0;
    UCB0CTL1 = (0 + UCSWRST*1 );

    // clearing IFg /16.4.9/p447/SLAU144j
    // set by setting UCSWRST just before
    IFG2 &= ~(UCB0TXIFG | UCB0RXIFG);

    // Configuration SPI (voir slau144 p.445)
    // UCCKPH = 0 -> Data changed on leading clock edges and sampled on trailing edges.
    // UCCKPL = 0 -> Clock inactive state is low.
    //   SPI Mode 0 :  UCCKPH * 1 | UCCKPL * 0
    //   SPI Mode 1 :  UCCKPH * 0 | UCCKPL * 0  <--
    //   SPI Mode 2 :  UCCKPH * 1 | UCCKPL * 1
    //   SPI Mode 3 :  UCCKPH * 0 | UCCKPL * 1
    // UCMSB  = 1 -> MSB premier
    // UC7BIT = 0 -> 8 bits, 1 -> 7 bits
    // UCMST  = 0 -> CLK by Master, 1 -> CLK by USCI bit CLK / p441/16.3.6
    // UCMODE_x  x=0 -> 3-pin SPI,
    //           x=1 -> 4-pin SPI UC0STE active high,
    //           x=2 -> 4-pin SPI UC0STE active low,
    //           x=3 -> i²c.
    // UCSYNC = 1 -> Mode synchrone (SPI)
    UCB0CTL0 |= ( UCMST | UCMODE_0 | UCSYNC );
    UCB0CTL0 &= ~( UCCKPH | UCCKPL | UCMSB | UC7BIT );
    UCB0CTL1 |= UCSSEL_2;

    UCB0BR0 = 0x0A;     // divide SMCLK by 10
    UCB0BR1 = 0x00;

    // SPI : Fonctions secondaires
    // MISO-1.6 MOSI-1.7 et CLK-1.5
    // Ref. SLAS735G p48,49
    P1SEL  |= ( SCK | DATA_OUT | DATA_IN);
    P1SEL2 |= ( SCK | DATA_OUT | DATA_IN);

    UCB0CTL1 &= ~UCSWRST;                                // activation USCI
}





/* ----------------------------------------------------------------------------
 * Fonction d'envoie d'un caractère sur USCI en SPI 3 fils MASTER Mode
 * Entree : Caractère à envoyer
 * Sorties: /
 */
void Send_char_SPI2(unsigned char carac)
{
    while ((UCB0STAT & UCBUSY));   // attend que USCI_SPI soit dispo.
    while(!(IFG2 & UCB0TXIFG)); // p442
    UCB0TXBUF = carac;              // Put character in transmit buffer
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

        case 'e':
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
            strcpy(stringPrint,"\tr: allumer/eteindre le servomoteur\n\r\tx: eteindre les capteurs et le servomoteur\n\r\te: afficher les valeurs de debug\n\r\th: afficher l'aide\n\r");
            while(stringPrint[i] != '\0')
            {
                while (!(IFG2 & UCA0TXIFG));  // USCI_A0 TX buffer ready?
                UCA0TXBUF = stringPrint[i];              // TX -> RXed character
                i++;
            }
            i = 0;
            strcpy(stringPrint,"\tespace: start/stop\n\r\tz: avancer\n\r\ts: reculer\n\r\tq: gauche\n\r\td: droite\n\r");
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
                Send_char_SPI2(E_ACTIVE);
                P1OUT |= BIT0; // debug instruction
            }
            else
            {
                strcat(stringPrint,"desactive\n\r");
                infrarouge.etat = E_DESACTIVE;
                Send_char_SPI2(E_DESACTIVE);
                P1OUT &= ~BIT0; // debug instruction
            }
            while(stringPrint[i] != '\0')
            {
                while (!(IFG2 & UCA0TXIFG));  // USCI_A0 TX buffer ready?
                UCA0TXBUF = stringPrint[i];              // TX -> RXed character
                i++;
            }
            break;

        case 's':
            Set_Direction(RECULER);
            break;

        case 'q':
            Set_Direction(GAUCHE);
            break;

        case 'd':
            Set_Direction(DROITE);
            break;

        case 0x20:
            if(TA1CCR1!=0)
            {
                Set_vitesse(0,0);
            }
            else
            {
                Set_vitesse(150,150);
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

        case 'r':
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

        case 'z':
            Set_Direction(AVANCER);
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

    Init_robot();
    init_USCI();
    /* Timer
    TA0CTL=0|(TASSEL_2|ID_3);//sourceSMCLK,pasdepredivisionID_0
    TA0CTL|=MC_3;//comptageenmodeupdown
    TA0CTL|=TAIE;//autorisationinterruptionTAIE
    TA0CCR0=62500;//periode timer pour 1s*/
    Set_Direction(AVANCER);

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

