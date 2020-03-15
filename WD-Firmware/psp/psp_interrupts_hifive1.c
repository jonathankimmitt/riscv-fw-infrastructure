/* 
* SPDX-License-Identifier: Apache-2.0
* Copyright 2019 Western Digital Corporation or its affiliates.
* 
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
* 
* http:*www.apache.org/licenses/LICENSE-2.0
* 
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/
/**
* @file   psp_interrupts_hifive.c
* @author Ronen Haen
* @date   20.05.2019
* @brief  The file supplies interrupt services such as  - registration, default handlers and indication, whether we are in interrupt context, etc.
*         The file is specific to hifive1 specifications
* 
*/

/**
* include files
*/
#include "psp_api.h"

/**
* definitions
*/

/* The stack used by interrupt service routines */
#if (0 == D_ISR_STACK_SIZE)
    #error "ISR Stack size is not defined"
#else
	static /*D_PSP_DATA_SECTION*/ D_PSP_16_ALIGNED pspStack_t udISRStack[ D_ISR_STACK_SIZE ] ;
	const pspStack_t xISRStackTop = ( pspStack_t ) &( udISRStack[ ( D_ISR_STACK_SIZE ) - 1 ] );
#endif

/**
* macros
*/

/**
* types
*/

/**
* local prototypes
*/

/* Default ISRs */
void pspDefaultExceptionIntHandler_isr(void);
void pspDefaultEmptyIntHandler_isr(void);

/* External interrupts functions (empty functions for HiFive) */
D_PSP_TEXT_SECTION void pspExternalInterruptDisableNumber(u32_t uiIntNum);
D_PSP_TEXT_SECTION void pspExternalInterruptEnableNumber(u32_t uiIntNum);
D_PSP_TEXT_SECTION void pspExternalInterruptSetPriority(u32_t uiIntNum, u32_t uiPriority);
D_PSP_TEXT_SECTION void pspExternalInterruptsSetThreshold(u32_t uiThreshold);
D_PSP_TEXT_SECTION pspInterruptHandler_t pspExternalInterruptRegisterISR(u32_t uiVectorNumber, pspInterruptHandler_t pIsr, void* pParameter);


/**
* external prototypes
*/

/**
* global variables
*/

/* Exception handlers */
D_PSP_DATA_SECTION pspInterruptHandler_t  g_fptrExceptions_ints[D_PSP_NUM_OF_INTS_EXCEPTIONS] = {
                       pspDefaultEmptyIntHandler_isr,
                       pspDefaultEmptyIntHandler_isr,
                       pspDefaultEmptyIntHandler_isr,
                       pspDefaultEmptyIntHandler_isr,
                       pspDefaultEmptyIntHandler_isr,
                       pspDefaultEmptyIntHandler_isr,
                       pspDefaultEmptyIntHandler_isr,
                       pspDefaultEmptyIntHandler_isr,
                       pspDefaultEmptyIntHandler_isr,
                       pspDefaultEmptyIntHandler_isr,
                       pspDefaultEmptyIntHandler_isr,
                       pspDefaultEmptyIntHandler_isr,
                       pspDefaultEmptyIntHandler_isr,
                       pspDefaultEmptyIntHandler_isr,
                       pspDefaultEmptyIntHandler_isr };

/* Exceptions handler pointer */
D_PSP_DATA_SECTION pspInterruptHandler_t g_fptrIntExceptionIntHandler   = pspDefaultExceptionIntHandler_isr;

/* Interrupts handler pointers */
D_PSP_DATA_SECTION pspInterruptHandler_t g_fptrIntSSoftIntHandler       = pspDefaultEmptyIntHandler_isr;
D_PSP_DATA_SECTION pspInterruptHandler_t g_fptrIntRsrvdSoftIntHandler   = pspDefaultEmptyIntHandler_isr;
D_PSP_DATA_SECTION pspInterruptHandler_t g_fptrIntMSoftIntHandler       = pspDefaultEmptyIntHandler_isr;
D_PSP_DATA_SECTION pspInterruptHandler_t g_fptrIntUTimerIntHandler      = pspDefaultEmptyIntHandler_isr;
D_PSP_DATA_SECTION pspInterruptHandler_t g_fptrIntSTimerIntHandler      = pspDefaultEmptyIntHandler_isr;
D_PSP_DATA_SECTION pspInterruptHandler_t g_fptrIntRsrvdTimerIntHandler  = pspDefaultEmptyIntHandler_isr;
D_PSP_DATA_SECTION pspInterruptHandler_t g_fptrIntMTimerIntHandler      = pspDefaultEmptyIntHandler_isr;
D_PSP_DATA_SECTION pspInterruptHandler_t g_fptrIntUExternIntHandler     = pspDefaultEmptyIntHandler_isr;
D_PSP_DATA_SECTION pspInterruptHandler_t g_fptrIntSExternIntHandler     = pspDefaultEmptyIntHandler_isr;
D_PSP_DATA_SECTION pspInterruptHandler_t g_fptrIntRsrvdExternIntHandler = pspDefaultEmptyIntHandler_isr;
D_PSP_DATA_SECTION pspInterruptHandler_t g_fptrIntMExternIntHandler     = pspDefaultEmptyIntHandler_isr;
D_PSP_DATA_SECTION pspInterruptHandler_t g_fptrIntUSoftIntHandler       = pspDefaultEmptyIntHandler_isr;


/* External-interrupts function pointers */
D_PSP_DATA_SECTION void (*g_fptrPspExternalInterruptDisableNumber)(u32_t uiIntNum)                 = pspExternalInterruptDisableNumber;
D_PSP_DATA_SECTION void (*g_fptrPspExternalInterruptEnableNumber)(u32_t uiIntNum)                  = pspExternalInterruptEnableNumber;
D_PSP_DATA_SECTION void (*g_fptrPspExternalInterruptSetPriority)(u32_t uiIntNum, u32_t uiPriority) = pspExternalInterruptSetPriority;
D_PSP_DATA_SECTION void (*g_fptrPspExternalInterruptSetThreshold)(u32_t uiThreshold)               = pspExternalInterruptsSetThreshold;
D_PSP_DATA_SECTION pspInterruptHandler_t (*g_fptrPspExternalInterruptRegisterISR)(u32_t uiVectorNumber, pspInterruptHandler_t pIsr, void* pParameter) = pspExternalInterruptRegisterISR;



/**
* The function installs an interrupt service routine per risc-v cause
*
* @param fptrInterruptHandler     – function pointer to the interrupt service routine
* @param uiInterruptCause           – interrupt source
*
* @return u32_t                   - previously registered ISR
*/
D_PSP_TEXT_SECTION pspInterruptHandler_t pspRegisterInterruptHandler(pspInterruptHandler_t fptrInterruptHandler, u32_t uiInterruptCause)
{
   pspInterruptHandler_t fptrFunc;

   M_PSP_ASSERT(fptrInterruptHandler == NULL && uiInterruptCause >= E_LAST_COMMON_CAUSE);

   switch (uiInterruptCause)
   {
      case E_USER_SOFTWARE_CAUSE:
    	  fptrFunc = g_fptrIntUSoftIntHandler;
    	  g_fptrIntUSoftIntHandler = fptrInterruptHandler;
    	  break;
      case E_SUPERVISOR_SOFTWARE_CAUSE:
    	  fptrFunc = g_fptrIntSSoftIntHandler;
    	  g_fptrIntSSoftIntHandler = fptrInterruptHandler;
          break;
      case E_RESERVED_SOFTWARE_CAUSE:
    	  fptrFunc = g_fptrIntRsrvdSoftIntHandler;
    	  g_fptrIntRsrvdSoftIntHandler = fptrInterruptHandler;
    	  break;
      case E_MACHINE_SOFTWARE_CAUSE:
    	  fptrFunc = g_fptrIntMSoftIntHandler;
    	  g_fptrIntMSoftIntHandler = fptrInterruptHandler;
          break;
      case E_USER_TIMER_CAUSE:
    	  fptrFunc = g_fptrIntUTimerIntHandler;
    	  g_fptrIntUTimerIntHandler = fptrInterruptHandler;
    	  break;
      case E_SUPERVISOR_TIMER_CAUSE:
    	  fptrFunc = g_fptrIntSTimerIntHandler;
    	  g_fptrIntSTimerIntHandler = fptrInterruptHandler;
    	  break;
      case E_RESERVED_TIMER_CAUSE:
    	  fptrFunc = g_fptrIntRsrvdTimerIntHandler;
    	  g_fptrIntRsrvdTimerIntHandler = fptrInterruptHandler;
    	  break;
      case E_MACHINE_TIMER_CAUSE:
    	  fptrFunc = g_fptrIntMTimerIntHandler;
    	  g_fptrIntMTimerIntHandler = fptrInterruptHandler;
    	  break;
      case E_USER_EXTERNAL_CAUSE:
    	  fptrFunc = g_fptrIntUExternIntHandler;
    	  g_fptrIntUExternIntHandler = fptrInterruptHandler;
          break;
      case E_SUPERVISOR_EXTERNAL_CAUSE:
    	  fptrFunc = g_fptrIntSExternIntHandler;
    	  g_fptrIntSExternIntHandler = fptrInterruptHandler;
          break;
      case E_RESERVED_EXTERNAL_CAUSE:
    	  fptrFunc = g_fptrIntRsrvdExternIntHandler;
    	  g_fptrIntRsrvdExternIntHandler = fptrInterruptHandler;
    	  break;
      case E_MACHINE_EXTERNAL_CAUSE:
    	  fptrFunc = g_fptrIntMExternIntHandler;
    	  g_fptrIntMExternIntHandler = fptrInterruptHandler;
    	  break;
      default:
    	  fptrFunc = NULL;
    	  break;
   }

   return fptrFunc;
}

/**
* The function installs an exception handler per exception cause
*
* @param fptrInterruptHandler     – function pointer to the exception handler
* @param uiExceptionCause           – exception cause
*
* @return u32_t                   - previously registered ISR
*/
D_PSP_TEXT_SECTION pspInterruptHandler_t pspRegisterExceptionHandler(pspInterruptHandler_t fptrInterruptHandler, u32_t uiExceptionCause)
{
   pspInterruptHandler_t fptrFunc;

   M_PSP_ASSERT(fptrInterruptHandler == NULL && uiExceptionCause >= E_EXC_LAST_COMMON);

   fptrFunc = g_fptrExceptions_ints[uiExceptionCause];

   g_fptrExceptions_ints[uiExceptionCause] = fptrInterruptHandler;

   return fptrFunc;
}

/**
* default exception interrupt handler
*
* @param none
*
* @return none
*/
D_PSP_TEXT_SECTION void pspDefaultExceptionIntHandler_isr(void)
{
   /* get the exception cause */
   u32_t uiCause = M_PSP_READ_CSR(D_PSP_MCAUSE_NUM);

   /* is it a valid cause */
   M_PSP_ASSERT(uiCause < D_PSP_NUM_OF_INTS_EXCEPTIONS);

   /* call the specific exception handler */
   g_fptrExceptions_ints[uiCause]();
}


/**
* default empty interrupt handler
*
* @param none
*
* @return none
*/
D_PSP_TEXT_SECTION void pspDefaultEmptyIntHandler_isr(void)
{
}


/**
* This function disables a specified external interrupt (empty function)
*/
D_PSP_TEXT_SECTION void pspExternalInterruptDisableNumber(u32_t uiIntNum)
{
}

/*
* This function enables a specified external interrupt (empty function)
*/
D_PSP_TEXT_SECTION void pspExternalInterruptEnableNumber(u32_t uiIntNum)
{
}

/*
*  This function sets the priority of a specified external interrupts (empty function)
*/
D_PSP_TEXT_SECTION void pspExternalInterruptSetPriority(u32_t uiIntNum, u32_t uiPriority)
{
}

/*
*  This function sets the priority threshold of the external interrupts (empty function)
*/
D_PSP_TEXT_SECTION void pspExternalInterruptsSetThreshold(u32_t uiThreshold)
{
}

/*
* This function register external function handler (empty function)
*/
D_PSP_TEXT_SECTION pspInterruptHandler_t pspExternalInterruptRegisterISR(u32_t uiVectorNumber, pspInterruptHandler_t pIsr, void* pParameter)
{
  return 0 ;
}


