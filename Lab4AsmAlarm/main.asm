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

LOW_PWM     .set    64
HIGH_PWM    .set    32

SWITCH_T    .set    7000
TUNE_MAX    .set    16

;------------------------------------------------------------------------------
            .text                           ; Progam Start
;------------------------------------------------------------------------------
RESET       mov.w   #0280h,SP               ; Initialize stackpointer
StopWDT     mov.w   #WDTPW+WDTHOLD,&WDTCTL  ; Stop WDT

SetupTA0    mov.w   #SWITCH_T-1,&TA0CCR0
            mov.w   #OUTMOD_3,&TA0CCTL1
            mov.w   #TASSEL_1+MC_1,&TA0CTL
            mov.w   #CCIE,&TA0CCTL0
            mov.w   #0b,R4                  ; Flags (bit 0 - tone, bit 1 - tuning)
            mov.w   #0b,R5                  ; Low tone value
            mov.w   #0b,R6                  ; High tone value

SetupP2     bis.b   #BIT2,&P2DIR            ; P2.2 output
            bis.b   #BIT2,&P2SEL            ; P2.2 TA1.1 output
            bic.b   #BIT2,&P2SEL2           ; P2.2 default function

SetupTA1    mov.w   #LOW_PWM-1,&TA1CCR0     ; PWM Period
            mov.w   #OUTMOD_7,&TA1CCTL1     ; TA1CCR1 reset/set
            mov.w   #LOW_PWM>>1,&TA1CCR1    ; TA1CCR1 PWM Duty Cycle (pwm / 2)
            mov.w   #TASSEL_1+MC_1,&TA1CTL  ; ACLK, upmode
				
Mainloop    bis     #GIE+CPUOFF+SCG1+SCG0,SR; Enter LPM3
            nop                             ; Required only for debugger
            ; read adc to R7, set either R6 or R5 to R7
            bit     #BIT1,R4                ; Get tuning flag (bit 1) (0 == high tone)
            jz      HIGH_TUNE
            mov.w   #0b,R6 ; move R7
            jmp Mainloop
HIGH_TUNE   mov.w   #0b,R5 ; move R7
            jmp Mainloop
                                            ;
;-------------------------------------------------------------------------------
TA0_ISR;    Toggle Tone
;-------------------------------------------------------------------------------
            xor     #BIT0,R4                ; Toggle tone flag (R4 XOR BIT0)
            jz      HIGH_SW                 ; Switch to high tone
            mov.w   #LOW_PWM-1,R8
            add.w   R6,R8
            mov.w   #LOW_PWM>>1,R9
            jmp     SET_PWM
HIGH_SW     mov.w   #HIGH_PWM-1,R8
            mov.w   #HIGH_PWM>>1,R9

SET_PWM     mov.w   R8,&TA1CCR0
            mov.w   R9,&TA1CCR1 ; REWORK THIS PART
            bic     #GIE+CPUOFF+SCG1+SCG0,SR; Exit LPM3
            reti                            ;		
                                            ;
;------------------------------------------------------------------------------
;           Interrupt Vectors
;------------------------------------------------------------------------------
            .sect   ".reset"                ; MSP430 RESET Vector
            .short  RESET                   ;
            .sect   ".int09"                ; Timer_A0 Vector
            .short  TA0_ISR
            .end

