/*
 * BOURG Benjamin
 * GONNET Alex
 *
 * Send throw SPI value
 */

#include <msp430g2231.h>

/* Function to init the send and receive char from/to SPI */
void init_send(){
    /*--------------- Secure mode*/
        P1SEL = 0x00;        /* GPIO */
        P1DIR = 0x00;         /* IN */

        /* led */
        P1DIR |=  BIT0;
        P1OUT &= ~BIT0;

        /* USI Config. for SPI 3 wires Slave Op.*/
        /* P1SEL Ref. p41,42 SLAS694J used by USIPEx*/
        USICTL0 |= USISWRST;
        USICTL1 = 0;

        /* 3 wire, mode Clk&Ph / 14.2.3 p400 */
        /* SDI-SDO-SCLK - LSB First - Output Enable - Transp. Latch */
        USICTL0 |= (USIPE7 | USIPE6 | USIPE5 | USILSB | USIOE | USIGE );
        /* Slave Mode SLAU144J 14.2.3.2 p400 */
        USICTL0 &= ~(USIMST);
        USICTL1 |= USIIE;
        USICTL1 &= ~(USICKPH | USII2C);

        USICKCTL = 0;           /* No Clk Src in slave mode */
        USICKCTL &= ~(USICKPL | USISWCLK);  /* Polarity - Input ClkLow */

        USICNT = 0;
        USICNT &= ~(USI16B | USIIFGCC ); /* Only lower 8 bits used 14.2.3.3 p 401 slau144j */
        USISRL = 0x23;  /* hash, just mean ready; USISRL Vs USIR by ~USI16B set to 0 */
        USICNT = 0x08;
}



/* Function to send char throw SPI*/
void Send_char_SPI(unsigned char carac)
{
    while (!(USIIFG & USICTL1));   /* attend que USCI_SPI soit dispo. */
    USISRL = carac;
    USICNT &= ~USI16B;
    USICNT = 8;
}

/* Function to send a string of char */
void Send_chaine_SPI(unsigned char *msg)
{
    unsigned int i = 0;
    for(i = 0; msg[i] != 0x00; i++)
    {
        Send_char_SPI(msg[i]);
    }
    Send_char_SPI('z');/* Caractère de fin de message */
}
