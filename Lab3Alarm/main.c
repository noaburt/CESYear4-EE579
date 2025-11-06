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

short tone_flag = 0;
short tune_flag = 0;


int main(void)
{
  WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT

  /* ADC Setup */
  ADC10CTL0 = SREF_1 + ADC10SHT_2 + REFON + ADC10ON + ADC10IE;
  __enable_interrupt();                     // Enable interrupts.
  TA1CCR0 = 30;                              // Delay to allow Ref to settle
  TA1CCTL0 |= CCIE;                          // Compare-mode interrupt.
  TA1CTL = TASSEL_2 | MC_1;                  // TACLK = SMCLK, Up mode.
  LPM0;                                     // Wait for delay.
  TA1CCTL0 &= ~CCIE;                         // Disable timer Interrupt
  __disable_interrupt();
  ADC10CTL1 = INCH_1;                       // input A1
  ADC10AE0 |= 0x02;                         // PA.1 ADC option select
  P1DIR |= 0x01;                            // Set P1.0 to output direction

  /* Switching Setup */
  TA0CCR0 = SWITCH_T-1;                          // Switching Period
  TA0CCTL1 = OUTMOD_3;                       // TACCR1 set/reset
  TA0CTL = TASSEL_1 + MC_1;                  // ACLK, up mode
  TA0CCTL0 |= CCIE;                          // Compare-mode interrupt

  /* Buzzer Setup */
  P2DIR |= BIT2;                            // P2.2 output - since LED is P2.1
  P2SEL |= BIT2;                            // P2.2 for TA1.1 output
  P2SEL2 &= ~(BIT2);								// Select default function for P2.1

  TA1CCR0 = 64-1;                               // PWM Period
  TA1CCTL1 = OUTMOD_7;                               // CCR1 reset/set
  TA1CCR1 = 32;                      // CCR1 PWM duty cycle
  TA1CTL = TASSEL_1 + MC_1;                        // ACLK, up mode

  for (;;)
  {
    ADC10CTL0 |= ENC + ADC10SC;             // Sampling and conversion start
    __bis_SR_register(LPM3_bits + GIE);        // LPM0, ADC10_ISR will force exi
    if (ADC10MEM < 0x88)                    // ADC10MEM = A1 > 0.2V?
      P1OUT &= ~0x01;                       // Clear P1.0 LED off
    else
      P1OUT |= 0x01;                        // Set P1.0 LED on
  }
}

// ADC10 interrupt service routine
#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR (void)
{
  LPM3_EXIT;
}

#pragma vector=TIMER1_A0_VECTOR
__interrupt void ta1_isr(void)
{
  LPM3_EXIT;
}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void TA0_ISR(void)
{
    if (tone_flag == 0) {
      tone_flag = 1;
      // switch to high
      TA1CCR0 = HIGH_PWM-1;
      TA1CCR1 = HIGH_PWM / DUTY_PWM;
    } else {
      tone_flag = 0;
      // switch to low
      TA1CCR0 = LOW_PWM-1;
      TA1CCR1 = LOW_PWM / DUTY_PWM;
    }

  LPM3_EXIT;
}