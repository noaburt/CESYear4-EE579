//******************************************************************************
//  MSP430G2xx3 Demo - Timer_A, Toggle P1.0, CCR0 Up Mode ISR, 32kHz ACLK
//
//  Description: Toggle P1.0 using software and the TA_0 ISR. Timer_A is
//  configured for up mode, thus the the timer overflows when TAR counts
//  to CCR0. In this example, CCR0 is loaded with 1000-1.
//  Toggle rate = 32768/24582 = 1,333Hz
//  ACLK = TACLK = 32768Hz, MCLK = SMCLK = DCO
//  //* An external watch crystal on XIN XOUT is required for ACLK *//	
//
//           MSP430G2xx3
//         ---------------
//     /|\|            XIN|-
//      | |               | 32kHz
//      --|RST        XOUT|-
//        |               |
//        |           P1.0|-->LED
//
//  D. Dang
//  Texas Instruments Inc.
//  December 2010
//   Built with CCS Version 4.2.0 and IAR Embedded Workbench Version: 5.10
//******************************************************************************

#include <msp430.h>

#define THIRTY_REPS 40 // 30s / 0.75s = 40

short count_flag;
int counter;

int main(void)
{
  WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT

  count_flag = 0;
  counter = THIRTY_REPS;   

  P1DIR |= BIT0;                            // P1.0 output
  P1OUT =  BIT3;                            // P1.3 set, else reset
  P1REN |= BIT3;                            // P1.3 pullup
  P1IE |= BIT3;                             // P1.3 interrupt enabled
  P1IES |= BIT3;                            // P1.3 Hi/lo edge
  P1IFG &= ~BIT3;                           // P1.3 IFG cleared

  CCTL0 = CCIE;                             // CCR0 interrupt enabled
  CCR0 = 25000-1;
  TACTL = TASSEL_1 + MC_1;                  // ACLK, upmode

  P1OUT |= BIT0;

  __bis_SR_register(LPM3_bits + GIE);       // Enter LPM3 w/ interrupt
}

// Timer A0 interrupt service routine
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A (void)
{
  if ( count_flag ) {    
    if ( --counter == 0 ) {
      count_flag = 0;
      P1OUT |= BIT0;
    }
    return;
  }

  P1OUT ^= BIT0;
}

// Port 1 interrupt service routine
#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
  counter = THIRTY_REPS;    // reset counter
  count_flag = 1;

  P1OUT &= ~BIT0;
  P1IFG &= ~BIT3;                           // P1.3 IFG cleared
}

