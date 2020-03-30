#include <msp430g2231.h>


void init_send(){
    P1OUT |= 0x06;
    P1DIR |= 0x06;


    // USI Config. for SPI 3 wires Slave Op.
    // P1SEL Ref. p41,42 SLAS694J used by USIPEx
    USICTL0 |= USISWRST;
    USICTL1 = 0;

    // 3 wire, mode Clk&Ph / 14.2.3 p400
    // SDI-SDO-SCLK - LSB First - Output Enable - Transp. Latch
   // USICTL0 |= (USIPE7 | USIPE6 | USIPE5 | USIMST | USIOE);

    USICTL1 |= USICKPH + USIIE;               // Counter interrupt, flag remains set
    USICKCTL = USIDIV_1 + USISSEL_2;          // /2 SMCLK

 /*   USICNT = 0;
    USICNT &= ~(USI16B | USIIFGCC ); // Only lower 8 bits used 14.2.3.3 p 401 slau144j
    USISRL = 0x23;  // hash, just mean ready; USISRL Vs USIR by ~USI16B set to 0
    USICNT = 0x08;
    */USICNT &= ~(USI16B | USIIFGCC );

    USISRL = 0x23;  // hash, just mean ready; USISRL Vs USIR by ~USI16B set to 0

   USICTL0 |= USIPE7 + USIPE6 + USIPE5 + USIMST + USIOE; // Port, SPI Master
    USICTL0 &= ~USISWRST;                     // USI released for operation
    USICNT = 8;

}




void Send_char_SPI(unsigned char carac)
{
    while (!(USIIFG & USICTL1));   // attend que USCI_SPI soit dispo.
    USISRL = carac;
    USICNT &= ~USI16B;
    USICNT = 8;
}
