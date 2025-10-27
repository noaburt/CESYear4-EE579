// Based on:
// MSP430G2xx3 Demo - Software TOggle P1.0
// by E. Chen, Texas Instruments Inc, May 2018

//******************************************************************************
//  MSP430G2xx3 Demo - Software Toggle P1.0
//
//  Description; Toggle P1.0 by xor'ing P1.0 inside of a software loop.
//  ACLK = n/a, MCLK = SMCLK = default DCO
//
//                MSP430G2xx3
//             -----------------
//         /|\|              XIN|-
//          | |                 |
//          --|RST          XOUT|-
//            |                 |
//            |             P1.0|-->LED
//
//  E. Chen
//  Texas Instruments, Inc
//  May 2018
//  Built with CCS Version 8.0 and IAR Embedded Workbench Version: 7.11
//******************************************************************************

#include <msp430.h>

#define THREE_SEC 100000000000
#define EIGHTY_PER 10000000

int main(void)
{
    volatile unsigned int i;
    volatile unsigned int j;
    volatile unsigned int state;

    volatile int button;

    WDTCTL = WDTPW + WDTHOLD;                 // Stop watchdog timer

    P1DIR |= BIT6;                            // Set P1.6 to output direction (Red LED)
    P2DIR |= BIT1 + BIT3 + BIT5;              // Set P2.1, P2.3, and P2.5 to output direction (Multi LED)

    P1OUT |= BIT6;                            // Pull up resistor on P1.6

    P2OUT &= ~(BIT1 + BIT3 + BIT5);           // Initialise Multi LED off
    P1OUT &= ~BIT6;

    state = 0;                                // Initialise state
    j = 0;

    while(1)
    {
      button = P1IN & BIT3;                   // Read P1.3 for button press

      switch ( state ) {
        case 0:
          j = THREE_SEC;

          if ( !button ) {                  // If logic is not LOW then button is released
            state = 1;
          }
          // nothing
          break;

        case 1:
          P2OUT |= BIT1 + BIT3;             // Set Multi LED to yellow
          j--;

          if ( j <= 1 ) {
            P2OUT &= ~(BIT1 + BIT3 + BIT5);
            P2OUT ^= BIT1;
            state = 2;
          }
          break;

        case 2:
          if ( button )                         // If button is released, reset state and LEDs
          {
            state = 0;
            P2OUT &= ~(BIT1 + BIT3 + BIT5);
            P1OUT &= ~BIT6;
            break;
          }

          P1OUT ^= BIT6;                    // Toggle P1.6 using exclusive-OR
          P2OUT ^= BIT1;                    // Toggle P2.1 for 2nd flashing red

          for (i = EIGHTY_PER; i > 0; i--); // Empty loop to delay just enough for 80 flashes / min
          break;
      }

      if ( button )                         // If button is released, reset state and LEDs
      {
        state = 0;
        P2OUT &= ~(BIT1 + BIT3 + BIT5);
        P1OUT &= ~BIT6;
      }
  }
}
