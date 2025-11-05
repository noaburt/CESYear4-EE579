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

#define LOW_PWM 1000
#define HIGH_PWM 2500

short f_flag = 0;

int main(void)
{
  float pwm_D = 0.5;
  int switch_T = 50000;

  WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT
  P1DIR |= BIT2;                            // P1.2 output
  P1SEL |= BIT2;                            // P1.2 for TA0.1 output
  P1SEL2 = 0;								// Select default function for P1.2

  /* PWM Timer */
  TA1CCR0 = pwm_T-1;                             // PWM Period
  TA1CCTL1 = OUTMOD_7;                           // CCR1 reset/set
  TA1CCR1 = pwm_T * pwm_D;                       // CCR1 PWM duty cycle
  TA1CTL = TASSEL_2 + MC_1;                      // SMCLK, up mode

  /* Switching Timer */
  TA0CCTL0 = CCIE;                             // TA0CCR0 interrupt enabled
  TA0CCR0 = switch_T-1;
  TA0CTL = TASSEL_1 + MC_1;

  __bis_SR_register(LPM3_bits + GIE);                // Enter LPM3
}


// TA0_A1 Interrupt vector
#pragma vector = TIMER0_A1_VECTOR
__interrupt void TIMER0_A1_ISR (void)
{
  // Switch high / low frequency

  if (f_flag == 0) {
    // switch to high
  } else {
    // switch to low
  }
}

