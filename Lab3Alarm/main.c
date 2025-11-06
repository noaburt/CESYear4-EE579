//******************************************************************************
//  MSP430G2xx3 Demo - Timer_A, PWM TA1-2, Up Mode, DCO SMCLK
//
//  Description: This program generates one PWM output on P1.2 using
//  Timer_A configured for up mode. The value in CCR0, 512-1, defines the PWM
//  period and the value in CCR1 the PWM duty cycles.
//  A 75% duty cycle on P1.2.
//  ACLK = na, SMCLK = MCLK = TACLK = default DCO
//
//               MSP430G2xx3
//            -----------------
//        /|\|              XIN|-
//         | |                 |
//         --|RST          XOUT|-
//           |                 |
//           |         P1.2/TA1|--> CCR1 - 75% PWM
//
//  Kathryn Adamsky						
//  Texas Instruments, Inc
//  June 2016 (updated) | December 2010 (created)
//  Built with CCS Version 4.2.0 and IAR Embedded Workbench Version: 5.10
//******************************************************************************

#include <msp430.h>

#define LOW_PWM 64
#define HIGH_PWM 32
#define DUTY_PWM 2

#define SWITCH_T 7000

short f_flag = 0;

int main(void)
{
  WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT

  P2DIR |= BIT2;                            // P2.1 output
  P2SEL |= BIT2;                            // P2.1 for TA1.1 output
  P2SEL2 &= ~(BIT2);								// Select default function for P2.1

  TA1CCR0 = LOW_PWM-1;                               // PWM Period
  TA1CCTL1 = OUTMOD_7;                               // CCR1 reset/set
  TA1CCR1 = LOW_PWM / DUTY_PWM;                      // CCR1 PWM duty cycle
  TA1CTL = TASSEL_1 + MC_1;                        // SMCLK, up mode

  TA0CCR0 = SWITCH_T-1;                          // PWM Period
  TA0CCTL1 = OUTMOD_3;                       // TACCR1 set/reset
  TA0CTL = TASSEL_1 + MC_1;                  // ACLK, up mode

  TA0CCTL0 |= CCIE;                          // Compare-mode interrupt.

  __bis_SR_register(LPM3_bits + GIE);
}


#pragma vector=TIMER0_A0_VECTOR
__interrupt void TA0_ISR(void)
{
  // Switch high / low frequency

  if (f_flag == 0) {
    // switch to high
    TA1CCR0 = HIGH_PWM-1;
    TA1CCR1 = HIGH_PWM / DUTY_PWM;
    f_flag = 1;
  } else {
    // switch to low
    TA1CCR0 = LOW_PWM-1;
    TA1CCR1 = LOW_PWM / DUTY_PWM;
    f_flag = 0;
  }
}