;/**************************************************************************//**
; * @file     startup_si91x.s
; * @brief    CMSIS Core Device Startup File for
; *           ARMCM4 Device Series
; * @version  V1.08
; * @date     23. November 2012
; *
; * @note
; *
; ******************************************************************************/
;/* Copyright (c) 2011 - 2012 ARM LIMITED
;
;   All rights reserved.
;   Redistribution and use in source and binary forms, with or without
;   modification, are permitted provided that the following conditions are met:
;   - Redistributions of source code must retain the above copyright
;     notice, this list of conditions and the following disclaimer.
;   - Redistributions in binary form must reproduce the above copyright
;     notice, this list of conditions and the following disclaimer in the
;     documentation and/or other materials provided with the distribution.
;   - Neither the name of ARM nor the names of its contributors may be used
;     to endorse or promote products derived from this software without
;     specific prior written permission.
;   *
;   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
;   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
;   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
;   ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDERS AND CONTRIBUTORS BE
;   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
;   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
;   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
;   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
;   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
;   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
;   POSSIBILITY OF SUCH DAMAGE.
;   ---------------------------------------------------------------------------*/
;/*
;//-------- <<< Use Configuration Wizard in Context Menu >>> ------------------
;*/


; <h> Stack Configuration
;   <o> Stack Size (in Bytes) <0x0-0xFFFFFFFF:8>
; </h>

Stack_Size      EQU     0x00000C00

                AREA    STACK, NOINIT, READWRITE, ALIGN=3
Stack_Mem       SPACE   Stack_Size
__initial_sp


; <h> Heap Configuration
;   <o>  Heap Size (in Bytes) <0x0-0xFFFFFFFF:8>
; </h>

Heap_Size       EQU     0x00000C00

                AREA    HEAP, NOINIT, READWRITE, ALIGN=3
__heap_base
Heap_Mem        SPACE   Heap_Size
__heap_limit


                PRESERVE8
                THUMB


; Vector Table Mapped to Address 0 at Reset

                AREA    RESET, DATA, READONLY
                EXPORT  __Vectors
                EXPORT  __Vectors_End
                EXPORT  __Vectors_Size
				IMPORT  RSI_PS_Restore	
				

__Vectors       DCD     0x2000;                   ; Bootloader stack pointer 
                DCD     0x300001;                 ; Boot loader entry address
                DCD     NMI_Handler               ; NMI Handler
                DCD     HardFault_Handler         ; Hard Fault Handler
                DCD     MemManage_Handler         ; MPU Fault Handler
                DCD     BusFault_Handler          ; Bus Fault Handler
                DCD     UsageFault_Handler        ; Usage Fault Handler
                DCD     0                         ; Reserved
                DCD     0                         ; Reserved
                DCD     0                         ; Reserved
                DCD     0                         ; Reserved
                DCD     SVC_Handler               ; SVCall Handler
                DCD     DebugMon_Handler          ; Debug Monitor Handler
                DCD     0                         ; Reserved
                DCD     PendSV_Handler            ; PendSV Handler
                DCD     SysTick_Handler           ; SysTick Handler

                ; External Interrupts
                DCD     IRQ000_Handler         	  ; 0: VAD interrupt  
				DCD     IRQ001_Handler			  ; 1: ULP Processor Interrupt1   
				DCD     IRQ002_Handler			  ; 2: ULP Processor Interrupt2   
				DCD		IRQ003_Handler			  ; 3: ULP Processor Interrupt3  
				DCD		IRQ004_Handler			  ; 4: ULP Processor Interrupt4  
			    DCD		IRQ005_Handler			  ; 5: ULP Processor Interrupt5 
			    DCD		IRQ006_Handler			  ; 6: ULP Processor Interrupt6 
			    DCD		IRQ007_Handler			  ; 7: ULP Processor Interrupt7 
				DCD		IRQ008_Handler			  ; 8: ULP Processor Interrupt8 
				DCD		IRQ009_Handler			  ; 9: ULP Processor Interrupt8 
				DCD		IRQ010_Handler			  ; 10: ULP Processor Interrupt8 
				DCD		IRQ011_Handler			  ; 11: ULP Processor Interrupt8 
                DCD     IRQ012_Handler			  ; 12: ULP Processor Interrupt8 
                DCD     IRQ013_Handler			  ; 13: ULP Processor Interrupt8 
                DCD     IRQ014_Handler			  ; 14: ULP Processor Interrupt8 
                DCD     IRQ015_Handler			  ; 15: ULP Processor Interrupt8 
                DCD     IRQ016_Handler			  ; 16: ULP Processor Interrupt8 
                DCD     IRQ017_Handler			  ; 17: ULP Processor Interrupt8 
                DCD     IRQ018_Handler			  ; 18: ULP Processor Interrupt8 
                DCD     IRQ019_Handler			  ; 19: ULP Processor Interrupt8 
				DCD     IRQ020_Handler			  ; 20: Sleep Sensor Interrupts 0
                DCD     IRQ021_Handler			  ; 21: Sleep Sensor Interrupts 1
                DCD     IRQ022_Handler			  ; 22: Sleep Sensor Interrupts 2
                DCD     IRQ023_Handler			  ; 23: Sleep Sensor Interrupts 3
                DCD     IRQ024_Handler			  ; 24: Sleep Sensor Interrupts 4
                DCD     IRQ025_Handler			  ; 25: Sleep Sensor Interrupts 5
				DCD     IRQ026_Handler			  ; 26: Sleep Sensor Interrupts 6
                DCD     IRQ027_Handler			  ; 27: Sleep Sensor Interrupts 7
				DCD     IRQ028_Handler			  ; 28: Sleep Sensor Interrupts 8
                DCD     IRQ029_Handler		      ; 29: Sleep Sensor Interrupts 9
                DCD     __initial_sp			  ; 30:	Reserved   
                DCD     IRQ031_Handler			  ; 31:	RPDMA interrupt
				DCD     Reset_Handler			  ; 32: Reserved                
				DCD     IRQ033_Handler			  ; 33:	UDMA interrupt
				DCD     IRQ034_Handler			  ; 34:	SCT interrupt
				DCD     HIF1_IRQHandler           ; 35: HIF Interrupt1
				DCD     HIF2_IRQHandler           ; 36: HIF Interrupt2
				DCD     IRQ037_Handler            ; 37: SIO Interrupt
				DCD     IRQ038_Handler 			  ; 38: USART 1 Interrupt
				DCD     IRQ039_Handler 			  ; 39: USART 2 Interrupt
				DCD     RSI_PS_Restore 	          ; 40: Reserved
				DCD     IRQ041_Handler 			  ; 41: GPIO WAKEUP INTERRUPT    
				DCD     IRQ042_Handler     		  ; 42: I2C Interrupt
				DCD     0x10AD10AD     			  ; 43: Reserved
				DCD     IRQ044_Handler     		  ; 44: SSI Slave Interrupt
				DCD     0     					  ; 45: Reserved
				DCD     IRQ046_Handler    		  ; 46: GSPI Master 1 Interrupt
				DCD     IRQ047_Handler			  ; 47: SSI Master Interrupt 
				DCD     IRQ048_Handler		  	  ; 48: MCPWM Interrupt
				DCD     IRQ049_Handler			  ; 49: QEI Interrupt              
				DCD     IRQ050_Handler			  ; 50: GPIO Group Interrupt 0    
				DCD     IRQ051_Handler			  ; 51: GPIO Group Interrupt 1    
				DCD     IRQ052_Handler		  	  ; 52: GPIO Pin Interrupt   0 
				DCD     IRQ053_Handler		  	  ; 53: GPIO Pin Interrupt   1 
				DCD     IRQ054_Handler		      ; 54: GPIO Pin Interrupt   2 
				DCD     IRQ055_Handler		  	  ; 55: GPIO Pin Interrupt   3 
				DCD     IRQ056_Handler	  		  ; 56: GPIO Pin Interrupt   4 
				DCD     IRQ057_Handler	  		  ; 57: GPIO Pin Interrupt   5 
				DCD     IRQ058_Handler	  		  ; 58: GPIO Pin Interrupt   6 
				DCD     IRQ059_Handler 			  ; 59: GPIO Pin Interrupt   7 
				DCD     IRQ060_Handler 			  ; 60: QSPI Interrupt 
				DCD     IRQ061_Handler 			  ; 61: I2C 2 Interrupt
				DCD     IRQ062_Handler 			  ; 62: Ethernet Interrupt
				DCD     IRQ063_Handler 			  ; 63: Ethernet PMT Interrupt
				DCD     IRQ064_Handler 			  ; 64: I2S master Interrupt
				DCD     0 						  ; 65: Reserved 
				DCD     IRQ066_Handler 			  ; 66: Can 1 Interrupt
				DCD     0 						  ; 67: Reserved
				DCD     IRQ068_Handler 			  ; 68: SDMEM Interrupt
				DCD     IRQ069_Handler 			  ; 69: PLL clock ind Interrupt
				DCD     0						  ; 70: Reserved
				DCD     IRQ071_Handler 			  ; 71: CCI system Interrupt Out
				DCD     IRQ072_Handler 			  ; 72: FPU exception
				DCD     IRQ073_Handler			  ; 73: USB INTR	
				DCD     IRQ074_Handler			  ; 74: TASS_P2P_INTR							
				DCD     IRQ075_Handler			  ; 75: WLAN Band1 intr0(NWP)
				DCD     IRQ076_Handler			  ; 76: WLAN Band1 intr1(NWP)
				DCD     0						  ; 77: Reserved(NWP)
				DCD     0						  ; 78: Reserved(NWP)
				DCD     IRQ079_Handler			  ; 79: BT intr(NWP)
				DCD     IRQ080_Handler			  ; 80: ZB intr(NWP)
				DCD     0						  ; 81: Reserved(NWP)
				DCD     IRQ082_Handler			  ; 82: Modem disabled mode trigger intr(NWP)
				DCD     IRQ083_Handler			  ; 83: gpio intr(NWP)
				DCD     IRQ084_Handler			  ; 84: uart intr(NWP)
				DCD     IRQ085_Handler			  ; 85: watch dog level intr(NWP)
				DCD     IRQ086_Handler			  ; 86: ULP Sleep sensor interrupt(NWP)
				DCD     IRQ087_Handler			  ; 87: ECDH intr(NWP)
				DCD     IRQ088_Handler			  ; 88: DH intr(NWP)
				DCD     IRQ089_Handler			  ; 89: QSPI intr(NWP)
				DCD     IRQ090_Handler			  ; 90: ULP processor interrupt TASS(NWP)
				DCD     IRQ091_Handler			  ; 91: Sys Tick Timer(NWP)
				DCD     IRQ092_Handler			  ; 92: Real Timer interrupt(NWP)
				DCD     IRQ093_Handler			  ; 93: PLL lock interrupt(NWP)
				DCD     0						  ; 94: Reserved(NWP)
				DCD     IRQ095_Handler			  ; 95: UART2 Interrupt(NWP)
				DCD     0						  ; 96: Reserved(NWP)
				DCD     IRQ097_Handler			  ; 97: I2C Interrupt(NWP)
					
					
__Vectors_End

__Vectors_Size  EQU     __Vectors_End - __Vectors


                AREA    |.text|, CODE, READONLY


; Reset Handler

Reset_Handler   PROC
                EXPORT  Reset_Handler             [WEAK]
                IMPORT  SystemInit
                IMPORT  __main


                LDR     R0, =SystemInit
                BLX     R0
                LDR     R0, =__main
                BX      R0
                ENDP

; Dummy Exception Handlers (infinite loops which can be modified)

NMI_Handler     PROC
                EXPORT  NMI_Handler               [WEAK]
                B       .
                ENDP
HardFault_Handler\
                PROC
                EXPORT  HardFault_Handler         [WEAK]
                B       .
                ENDP
MemManage_Handler\
                PROC
                EXPORT  MemManage_Handler         [WEAK]
                B       .
                ENDP
BusFault_Handler\
                PROC
                EXPORT  BusFault_Handler          [WEAK]
                B       .
                ENDP
UsageFault_Handler\
                PROC
                EXPORT  UsageFault_Handler        [WEAK]
                B       .
                ENDP
SVC_Handler     PROC
                EXPORT  SVC_Handler               [WEAK]
                B       .
                ENDP
DebugMon_Handler\
                PROC
                EXPORT  DebugMon_Handler          [WEAK]
                B       .
                ENDP
PendSV_Handler  PROC
                EXPORT  PendSV_Handler            [WEAK]
                B       .
                ENDP
SysTick_Handler PROC
                EXPORT  SysTick_Handler           [WEAK]
                B       .
                ENDP
HIF1_IRQHandler PROC
                EXPORT  HIF1_IRQHandler           [WEAK]
                B       .
                ENDP

HIF2_IRQHandler PROC
                EXPORT  HIF2_IRQHandler           [WEAK]
                B       .
                ENDP

TASS_P2P_INTR PROC
                EXPORT  TASS_P2P_INTR             [WEAK]
                B       .
                ENDP
					
Default_Handler PROC

                EXPORT  IRQ000_Handler              		[WEAK]
                EXPORT  IRQ001_Handler                		[WEAK]
                EXPORT  IRQ002_Handler                		[WEAK]
                EXPORT  IRQ003_Handler         		    	[WEAK]
                EXPORT  IRQ004_Handler                  	[WEAK]
                EXPORT  IRQ005_Handler                  	[WEAK]
                EXPORT  IRQ006_Handler                		[WEAK]
                EXPORT  IRQ007_Handler               		[WEAK]
                EXPORT  IRQ008_Handler                		[WEAK]
                EXPORT  IRQ009_Handler               		[WEAK]
                EXPORT  IRQ010_Handler               		[WEAK]
                EXPORT  IRQ011_Handler               		[WEAK]
                EXPORT  IRQ012_Handler               		[WEAK]
                EXPORT  IRQ013_Handler               		[WEAK]
				EXPORT  IRQ014_Handler                 		[WEAK]
                EXPORT  IRQ015_Handler                		[WEAK]
                EXPORT  IRQ016_Handler           			[WEAK]
				EXPORT  IRQ017_Handler           			[WEAK]
                EXPORT  IRQ018_Handler         				[WEAK]
                EXPORT  IRQ019_Handler         				[WEAK]
                EXPORT  IRQ020_Handler                    	[WEAK]
                EXPORT  IRQ021_Handler          			[WEAK]
                EXPORT  IRQ022_Handler                  	[WEAK]
                EXPORT  IRQ023_Handler                    	[WEAK]
                EXPORT  IRQ024_Handler                    	[WEAK]
                EXPORT  IRQ025_Handler                  	[WEAK]
                EXPORT  IRQ026_Handler             			[WEAK]
                EXPORT  IRQ027_Handler                    	[WEAK]
                EXPORT  IRQ028_Handler         				[WEAK]
                EXPORT  IRQ029_Handler       				[WEAK]
                EXPORT  IRQ031_Handler          			[WEAK]
                EXPORT  IRQ033_Handler          			[WEAK]
                EXPORT  IRQ034_Handler          			[WEAK]
				EXPORT  IRQ035_Handler          			[WEAK]
                EXPORT  IRQ036_Handler          			[WEAK]
                EXPORT  IRQ037_Handler          			[WEAK]
				EXPORT  IRQ038_Handler          			[WEAK]
			    EXPORT  IRQ039_Handler          			[WEAK]
		        EXPORT  IRQ041_Handler          			[WEAK]
                EXPORT  IRQ042_Handler          			[WEAK]
                EXPORT  IRQ044_Handler          			[WEAK]
				EXPORT  IRQ046_Handler          			[WEAK]
				EXPORT  IRQ047_Handler          			[WEAK]
                EXPORT  IRQ048_Handler          			[WEAK]
                EXPORT  IRQ049_Handler          			[WEAK]
                EXPORT  IRQ050_Handler          			[WEAK] 
				EXPORT  IRQ051_Handler          			[WEAK]
				EXPORT  IRQ052_Handler          			[WEAK] 
				EXPORT  IRQ053_Handler          			[WEAK]
				EXPORT  IRQ054_Handler          			[WEAK]
				EXPORT  IRQ055_Handler          			[WEAK]
				EXPORT  IRQ056_Handler          			[WEAK]   
				EXPORT  IRQ057_Handler          			[WEAK]   
				EXPORT  IRQ058_Handler          			[WEAK]   
				EXPORT  IRQ059_Handler          			[WEAK]
				EXPORT  IRQ060_Handler          			[WEAK]
				EXPORT  IRQ061_Handler          			[WEAK]   
				EXPORT  IRQ062_Handler          			[WEAK]   
				EXPORT  IRQ064_Handler          			[WEAK]   
				EXPORT  IRQ066_Handler          			[WEAK]   
				EXPORT  IRQ068_Handler          			[WEAK]   
				EXPORT  IRQ069_Handler          			[WEAK]
				EXPORT  IRQ071_Handler          			[WEAK]
				EXPORT  IRQ072_Handler          			[WEAK] 
				EXPORT  IRQ073_Handler          			[WEAK]
				EXPORT  IRQ074_Handler          			[WEAK] 
				EXPORT  IRQ075_Handler          			[WEAK] 
				EXPORT  IRQ076_Handler          			[WEAK] 
				EXPORT  IRQ079_Handler          			[WEAK] 
				EXPORT  IRQ080_Handler          			[WEAK] 
				EXPORT  IRQ082_Handler          			[WEAK] 
				EXPORT  IRQ083_Handler          			[WEAK] 
				EXPORT  IRQ084_Handler          			[WEAK] 
				EXPORT  IRQ085_Handler          			[WEAK] 
				EXPORT  IRQ086_Handler          			[WEAK] 
				EXPORT  IRQ087_Handler          			[WEAK] 
				EXPORT  IRQ088_Handler          			[WEAK]
				EXPORT  IRQ089_Handler          			[WEAK]
				EXPORT  IRQ090_Handler          			[WEAK]
				EXPORT  IRQ091_Handler          			[WEAK]
				EXPORT  IRQ092_Handler          			[WEAK]
				EXPORT  IRQ093_Handler          			[WEAK] 
				EXPORT  IRQ095_Handler          			[WEAK] 
				EXPORT  IRQ097_Handler          			[WEAK] 
			    EXPORT  wakeup          			        [WEAK] 
				 
               

IRQ000_Handler              		
IRQ001_Handler                		
IRQ002_Handler                		
IRQ003_Handler         		    	
IRQ004_Handler                  	
IRQ005_Handler                  	
IRQ006_Handler                		
IRQ007_Handler               		
IRQ008_Handler                		
IRQ009_Handler               		
IRQ010_Handler               		
IRQ011_Handler               		
IRQ012_Handler               		
IRQ013_Handler               	 
IRQ014_Handler                 		 
IRQ015_Handler                		 
IRQ016_Handler           		 
IRQ017_Handler           			 
IRQ018_Handler         				 
IRQ019_Handler         				 
IRQ020_Handler                     
IRQ021_Handler          			 
IRQ022_Handler                  	 
IRQ023_Handler                    	 
IRQ024_Handler                    	 
IRQ025_Handler                  	 
IRQ026_Handler             			 
IRQ027_Handler                     
IRQ028_Handler         				 
IRQ029_Handler       				 
IRQ031_Handler          			 
IRQ033_Handler          			 
IRQ034_Handler          			 
IRQ035_Handler          			 
IRQ036_Handler          			 
IRQ037_Handler          			 
IRQ038_Handler   
IRQ039_Handler   
IRQ041_Handler   
IRQ042_Handler 
IRQ043_Handler
IRQ044_Handler          			 
IRQ046_Handler 
IRQ047_Handler
IRQ048_Handler          			 
IRQ049_Handler          			 
IRQ050_Handler          			 
IRQ051_Handler          			 
IRQ052_Handler          			 
IRQ053_Handler          			 
IRQ054_Handler          			 
IRQ055_Handler          			 
IRQ056_Handler          			
IRQ057_Handler          			 
IRQ058_Handler          			 
IRQ059_Handler          			
IRQ060_Handler          			 
IRQ061_Handler          			 
IRQ062_Handler    
IRQ063_Handler    
IRQ064_Handler          			  
IRQ066_Handler          			
IRQ068_Handler          			   
IRQ069_Handler          			 
IRQ071_Handler          			 
IRQ072_Handler          			 
IRQ073_Handler          			
IRQ074_Handler          			
IRQ075_Handler          			 
IRQ076_Handler          			
IRQ079_Handler          			
IRQ080_Handler          			 
IRQ082_Handler          			
IRQ083_Handler          			
IRQ084_Handler          			
IRQ085_Handler          			
IRQ086_Handler          			
IRQ087_Handler          		
IRQ088_Handler          			
IRQ089_Handler          			 
IRQ090_Handler          			
IRQ091_Handler          			
IRQ092_Handler          			
IRQ093_Handler          		
IRQ095_Handler          		
IRQ097_Handler 
wakeup
 		
     B       .

     ENDP

     ALIGN


; User Initial Stack & Heap

                IF      :DEF:__MICROLIB

                EXPORT  __initial_sp
                EXPORT  __heap_base
                EXPORT  __heap_limit

                ELSE

                IMPORT  __use_two_region_memory
                EXPORT  __user_initial_stackheap

__user_initial_stackheap PROC
                LDR     R0, =  Heap_Mem
                LDR     R1, = (Stack_Mem + Stack_Size)
                LDR     R2, = (Heap_Mem +  Heap_Size)
                LDR     R3, = Stack_Mem
                BX      LR
                ENDP

                ALIGN

                ENDIF


                END
