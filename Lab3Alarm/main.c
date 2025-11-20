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
#include <math.h>

#define LOW_PWM 80
#define HIGH_PWM 48
#define DUTY_PWM 0.5

#define SWITCH_T 7000

unsigned short tone_flag = 0;
unsigned short tune_flag = 0;

double tune_val[2] = {0};

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

  /* Tuning Button Setup */
  P1DIR |= BIT0;                            // P1.0 output
  P1OUT =  BIT3;                            // P1.3 set, else reset
  P1REN |= BIT3;                            // P1.3 pullup
  P1IE |= BIT3;                             // P1.3 interrupt enabled
  P1IES |= BIT3;                            // P1.3 Hi/lo edge
  P1IFG &= ~BIT3;                           // P1.3 IFG cleared

  /* Switching Setup */
  TA0CCR0 = SWITCH_T-1;                          // Switching Period
  TA0CCTL1 = OUTMOD_3;                       // TACCR1 set/reset
  TA0CTL = TASSEL_1 + MC_1;                  // ACLK, up mode
  TA0CCTL0 |= CCIE;                          // Compare-mode interrupt

  /* Buzzer Setup */
  P2DIR |= BIT2;                            // P2.2 output - since LED is P2.1
  P2SEL |= BIT2;                            // P2.2 for TA1.1 output
  P2SEL2 &= ~(BIT2);								// Select default function for P2.2

  /* PWM Setup */
  TA1CCR0 = 64-1;                               // PWM Period
  TA1CCTL1 = OUTMOD_7;                               // CCR1 reset/set
  TA1CCR1 = 32;                      // CCR1 PWM duty cycle
  TA1CTL = TASSEL_1 + MC_1;                        // ACLK, up mode

  int adc_read;
  double tune_calc;

  for (;;)
  {
    ADC10CTL0 |= ENC + ADC10SC;             // Sampling and conversion start
    __bis_SR_register(LPM3_bits + GIE);
    adc_read = ADC10MEM;

    /* Map adc read value from 0 - 3.3v to -16 - 16 */
    tune_calc = ( (float)adc_read / 0x3FF ) * 0x20; // ( adc_read / 1023 ) * 32
    tune_val[tune_flag] = -16 + floor(tune_calc);
  }
}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void TA0_ISR(void)
{
    if (tone_flag == 0) {
      // switch to high
      TA1CCR0 = (HIGH_PWM-tune_val[0]) - 1;
      TA1CCR1 = (HIGH_PWM-tune_val[0]) * DUTY_PWM;
    } else {
      // switch to low
      TA1CCR0 = (LOW_PWM-tune_val[1]) - 1;
      TA1CCR1 = (LOW_PWM-tune_val[1]) * DUTY_PWM;
    }

  tone_flag = ~(tone_flag);
  LPM3_EXIT;
}

#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
  if (tune_flag == 0) {
    tune_flag = 1;
  } else {
    tune_flag = 0;
  }

  P1IFG &= ~BIT3;
}

#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR (void)
{
  LPM3_EXIT;
}

#pragma vector=TIMER1_A0_VECTOR
__interrupt void TA1_ISR(void)
{
  LPM3_EXIT;
}
