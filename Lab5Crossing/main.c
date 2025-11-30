//******************************************************************************
//  MSP430G2xx3 Demo - Timer_A, PWM TA1, Up Mode, 32kHz ACLK
//
//  Description: This program generates one PWM output on P1.2 using
//  Timer_A configured for up mode. The value in CCR0, 512-1, defines the PWM
//  period and the value in CCR1 the PWM duty cycles. Using 32kHz ACLK
//  as TACLK, the timer period is 15.6ms with a 75% duty cycle on P1.2.
//  Normal operating mode is LPM3.
//  ACLK = TACLK = LFXT1 = 32768Hz, MCLK = SMCLK = default DCO.
//  //* External watch crystal installed on XIN XOUT is required for ACLK *//	
//
//               MSP430G2xx3
//            -----------------
//        /|\|              XIN|-
//         | |                 | 32kHz
//         --|RST          XOUT|-
//           |                 |
//           |         P1.2/TA1|--> CCR1 - 75% PWM
//
//  D. Dang
//  Texas Instruments, Inc
//  December 2010
//   Built with CCS Version 4.2.0 and IAR Embedded Workbench Version: 5.10
//******************************************************************************

// Buzzer - GND, P2.2

#include <msp430.h>

#define KLAX_LOW 64
#define KLAX_HIGH 32
#define KLAX_DUTY 1   // Shift >> 1 == / 2 == 50% duty

/* LED Flash Logic
 *  - 5% duty is 1 on per 20 off, 50% is 10 on per 20 off
 *  - 30 flash per min = 2 flash per sec = TIMER VALUE ---------------------- TODO FIND VALUE
 *  - 2 flash per sec * 20 checks per "flash" = 40 checks per sec
 *  - 32768Hz clock, require period of 25ms, 0.025 * 32768 = 819.2 
 */

#define LED_FLASH 820
#define LED_TOTAL 20

unsigned short tone_flag;
unsigned short driver_flag;

unsigned short led_duty_count;
unsigned short driver_count;

unsigned short sw_read;

static unsigned short led_duty_amount[2] = {1, 10};

int main(void)
{
  WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT

  /* Buzzer */
  P2DIR |= BIT2;
  P2SEL |= BIT2;
  P2SEL2 &= ~(BIT2);

  TA1CCR0 = KLAX_LOW-1;
  TA1CCTL1 = OUTMOD_7;                      // TA0CCR1 reset/set
  TA1CCR1 = KLAX_LOW >> KLAX_DUTY;
  TA1CTL = TASSEL_1 + MC_1;                 // ACLK, up mode

  /* Driver LED */
  P1DIR |= BIT0;
  P1OUT &= ~(BIT0);

  TA0CCR0 = LED_FLASH-1;
  TA0CCTL1 = OUTMOD_3;
  TA0CTL = TASSEL_1 + MC_1;
  TA0CCTL0 |= CCIE;                          // Compare-mode interrupt

  /* Red LEDs */
  P1DIR |= BIT6;
  P2DIR |= BIT1 + BIT3 + BIT5;              // Set P2.1, P2.3, and P2.5 to output direction (Multi LED)

  P1OUT &= ~BIT6;
  P2OUT &= ~(BIT1 + BIT3 + BIT5);

  /* Button Setup */
  P1REN |= BIT3;                            // P1.3 pullup

  tone_flag = 0;
  led_duty_count = 0;
  driver_count = 0;
  driver_flag = 0;

  unsigned short check;

  for (;;) {
    __bis_SR_register(LPM3_bits + GIE);

    sw_read = P1IN & BIT3;

    /* Driver flag = button pressed for 3 seconds */
    if (sw_read > 0) {
      driver_flag = 0;
      driver_count = 0;
      P1OUT &= ~BIT6;
      P2OUT &= ~BIT1;
    }
    
    if (driver_count < LED_TOTAL*5) {
      driver_count++;
    } else {
      driver_flag = 1;
    }

    /* Set led on / off */
    check = (sw_read == 0 && driver_flag == 1);

    if (led_duty_count++ <= led_duty_amount[check]) { // Button pressed -> sw_read = 1 -> 50% duty
      /* 1st part of duty, LED on */
      P1OUT |= BIT0;
    } else {
      /* 2nd part of duty, LED off */
      P1OUT &= ~BIT0;
    }

    /* Toggle tone and reds */
    if (led_duty_count > LED_TOTAL) {
      led_duty_count = 0;

      if (tone_flag == 0) {
        // switch to high
        TA1CCR0 = KLAX_HIGH - 1;
        TA1CCR1 = KLAX_HIGH >> KLAX_DUTY;

        if (check > 0) {
          // multi led on, red off
          P2OUT |= BIT1;
          P1OUT &= ~BIT6;
        }
      } else {
        // switch to low
        TA1CCR0 = KLAX_LOW - 1;
        TA1CCR1 = KLAX_LOW >> KLAX_DUTY;

        if (check > 0) {
          // red led on, multi off
          P1OUT |= BIT6;
          P2OUT &= ~BIT1;
        }
      }

      tone_flag = ~(tone_flag);
    }
  }
}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void TA0_ISR(void)
{
  LPM3_EXIT;
}
