;******************************************************************************
;  
;                        MSP430 CODE EXAMPLE DISCLAIMER
; 
;  MSP430 code examples are self-contained low-level programs that typically
;  demonstrate a single peripheral function or device feature in a highly
;  concise manner. For this the code may rely on the device's power-on default
;  register values and settings such as the clock configuration and care must
;  be taken when combining code from several examples to avoid potential side
;  effects. Also see www.ti.com/grace for a GUI- and www.ti.com/msp430ware
;  for an API functional library-approach to peripheral configuration.
; 
; --/COPYRIGHT--
;*******************************************************************************
;   MSP430G2xx3 Demo - Timer_A, PWM TA1, Up Mode, DCO SMCLK
;
;   Description: This program generates one PWM output on P1.2 using
;   Timer_A configured for up mode. The value in CCR0, 512-1, defines the PWM
;   period and the value in CCR1 the PWM duty cycles.
;   A 75% duty cycle is on P1.2.
;   ACLK = n/a, SMCLK = MCLK = TACLK = default DCO
;
;                MSP430G2xx3
;             -----------------
;         /|\|              XIN|-
;          | |                 |
;          --|RST          XOUT|-
;            |                 |
;            |         P1.2/TA1|--> CCR1 - 75% PWM
;
;   D. Dang
;   Texas Instruments Inc.
;   December 2010
;   Built with Code Composer Essentials Version: 4.2.0
;*******************************************************************************
 .cdecls C,LIST,  "msp430.h"
;-------------------------------------------------------------------------------
            .def    RESET                   ; Export program entry-point to
                                            ; make it known to linker.

LOW_PWM     .set    80-16
HIGH_PWM    .set    48-16

SWITCH_T    .set    7000

;------------------------------------------------------------------------------
            .text                           ; Progam Start
;------------------------------------------------------------------------------
RESET       mov.w   #0280h,SP               ; Initialize stackpointer
StopWDT     mov.w   #WDTPW+WDTHOLD,&WDTCTL  ; Stop WDT

SetupADC10  mov.w   #SREF_1+ADC10SHT_2+REFON+ADC10ON+ADC10IE,&ADC10CTL0
            mov.w   #INCH_1, &ADC10CTL1
            bis.b   #2h,&ADC10AE0          ; P1.1 ADC10 option select
            nop

SetupP1     bis.b   #01h,&P1DIR            ; P1.0 output, else input
            mov.b   #BIT3,&P1OUT            ; P1.3 set, else reset
            bis.b   #BIT3,&P1REN            ; P1.3 pullup
            bis.b   #BIT3,&P1IE             ; P1.3 Interrupt enabled
            bis.b   #BIT3,&P1IES            ; P1.3 hi/low edge
            bic.b   #BIT3,&P1IFG            ; P1.3 IFG Cleared

SetupTA0    mov.w   #SWITCH_T-1,&TA0CCR0
            mov.w   #OUTMOD_3,&TA0CCTL1
            mov.w   #TASSEL_1+MC_1,&TA0CTL
            bis.w   #CCIE,&TA0CCTL0
            mov.w   #0b,R4                  ; Tone flag
            mov.w   #16d,R5                 ; Low tone value
            mov.w   #16d,R6                 ; High tone value
            mov.w   #0b,R7                  ; Tuning flag
            mov.w   #0b,R8                  ; Set tone

SetupP2     bis.b   #BIT2,&P2DIR            ; P2.2 output
            bis.b   #BIT2,&P2SEL            ; P2.2 TA1.1 output
            bic.b   #BIT2,&P2SEL2           ; P2.2 default function

SetupTA1    mov.w   #LOW_PWM-1,&TA1CCR0     ; PWM Period
            mov.w   #OUTMOD_7,&TA1CCTL1     ; TA1CCR1 reset/set
            mov.w   #LOW_PWM>>1,&TA1CCR1    ; TA1CCR1 PWM Duty Cycle (pwm / 2)
            mov.w   #TASSEL_1+MC_1,&TA1CTL  ; ACLK, upmode
				
Mainloop    bis.w   #ENC+ADC10SC,&ADC10CTL0 ; Start sampling/conversion
            bis     #GIE+CPUOFF+SCG1+SCG0,SR; Enter LPM3
            nop                             ; Required only for debugger

TuneTone    ; read adc to R8, set either R6 or R5 to R8 - &ADC10MEM
            mov.w   &ADC10MEM,R8            ; tune value = (adc / 1023) * 32)
            ; / 1023 x 32 === x 64 /2^16 x 32 === << 6, >> 16, << 5 === >> 5
            clrc
            rrc.w   R8
            clrc
            rrc.w   R8
            clrc
            rrc.w   R8
            clrc
            rrc.w   R8
            clrc
            rrc.w   R8
            clrc

            cmp     #1b,R7                ; Get tuning flag (bit 1) (0 == high tone)
            jz      TuneHigh

TuneLow     mov.w   R8,R5 ; move to R5 to change low
            jmp Mainloop
            nop

TuneHigh    mov.w   R8,R6 ; move to R6 to change high
            jmp Mainloop
            nop
                                            ;
                                            ;
                                            ;
;-------------------------------------------------------------------------------
TA0_ISR;    TA0 Interrupt - Toggle Tone
;-------------------------------------------------------------------------------
CheckTone   cmp.b   #1b,R4                  ; Tone flag check (1 - R4 = 0 -> high tone)
            jz      SetHigh
            nop

SetLow      mov.w   #LOW_PWM-1,R8
            add.w   R5,R8
            mov.w   #1b,R4
            jmp     SetPwm
            nop

SetHigh     mov.w   #HIGH_PWM-1,R8
            add.w   R6,R8
            mov.w   #0b,R4

SetPwm      mov.w   R8,&TA1CCR0
            add.w   R5,R8
            rrc     R8                      ; Divide by 2
            mov.w   R8,&TA1CCR1             ; 50% duty cycle

EndISR      bic     #CPUOFF+SCG1+SCG0,0(SP); Exit LPM3
            reti                            ;		
                                            ;
;-------------------------------------------------------------------------------
ADC10_ISR;  ADC Interrupt - Exit LPM3 on reti
;-------------------------------------------------------------------------------
            bic     #CPUOFF+SCG1+SCG0,0(SP); Exit LPM3
            reti                            ;
                                            ;
;-------------------------------------------------------------------------------
P1_ISR;     Toggle P1.0 Output
;-------------------------------------------------------------------------------
            xor.b   #1b,R7            ; Tune flag toggle
            bic.b   #BIT3,&P1IFG            ; P1.3 IFG Cleared
            reti                            ; Return from ISR
                                            ;
;-------------------------------------------------------------------------------
TA1_ISR;  TA1 Interrupt - Exit LPM3 on reti
;-------------------------------------------------------------------------------
            bic     #CPUOFF+SCG1+SCG0,0(SP); Exit LPM3
            reti                            ;
                                            ;
;------------------------------------------------------------------------------
;           Interrupt Vectors
;------------------------------------------------------------------------------
            .sect   ".reset"                ; MSP430 RESET Vector
            .short  RESET                   ;
            .sect   ".int02"                ; P1 Vector
            .short  P1_ISR
            .sect   ".int05"                ; ADC10 Vector
            .short  ADC10_ISR
            .sect   ".int09"                ; Timer_A0 Vector
            .short  TA0_ISR
            .sect   ".int08"                ; Timer_A1 Vector
            .short  TA1_ISR
            .end

