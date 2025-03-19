
#include "HardFaultHandler.hpp"
#include "Stm32ItmLoggerCPPWrapper.hpp"

#define printf Logger_printf

void ReportHardFault(uint32_t *stack_frame, uint32_t exc)
{
  uint32_t r0   = stack_frame[0];
  uint32_t r1   = stack_frame[1];
  uint32_t r2   = stack_frame[2];
  uint32_t r3   = stack_frame[3];
  uint32_t r12  = stack_frame[4];
  uint32_t lr   = stack_frame[5];
  uint32_t pc   = stack_frame[6];
  uint32_t psr  = stack_frame[7];
  uint32_t hfsr = SCB->HFSR;
  uint32_t cfsr = SCB->CFSR;
  uint32_t mmar = SCB->MMFAR;
  uint32_t bfar = SCB->BFAR;
  uint32_t afsr = SCB->AFSR;

  printf("\n!!!Hard Fault detected!!!\n");

  printf("\nStack frame:\n");
  printf("R0 :        0x%08lX\n", r0);
  printf("R1 :        0x%08lX\n", r1);
  printf("R2 :        0x%08lX\n", r2);
  printf("R3 :        0x%08lX\n", r3);
  printf("R12:        0x%08lX\n", r12);
  printf("LR :        0x%08lX\n", lr);
  printf("PC :        0x%08lX\n", pc);
  printf("PSR:        0x%08lX\n", psr);

  printf("\nFault status:\n");
  printf("HFSR:       0x%08lX\n", hfsr);
  printf("CFSR:       0x%08lX\n", cfsr);
  printf("MMAR:       0x%08lX\n", mmar);
  printf("BFAR:       0x%08lX\n", bfar);
  printf("AFSR:       0x%08lX\n", afsr);

  printf("\nOther:\n");
  printf("EXC_RETURN: 0x%08lX\n", exc);

  printf("\nDetails of the fault status:\n");
  printf("Hard fault status:\n");
  if (CHECK_BIT(hfsr, FORCED))
  {
    printf(" - Forced Hard fault.\n");
  }
  if (CHECK_BIT(hfsr, VECTTBL))
  {
    printf(" - Bus fault on vector table read.\n");
  }
  printf("MemManage fault status:\n");
  if (CHECK_BIT(cfsr, MMARVALID))
  {
    printf(" - MMAR holds a valid address.\n");
  }
  else
  {
    printf(" - MMAR holds an invalid address.\n");
  }
  if (CHECK_BIT(cfsr, MLSPERR))
  {
    printf(" - Fault occurred during floating-point lazy state preservation.\n");
  }
  if (CHECK_BIT(cfsr, MSTKERR))
  {
    printf(" - Stacking has caused an access violation.\n");
  }
  if (CHECK_BIT(cfsr, MUNSTKERR))
  {
    printf(" - Unstacking has caused an access violation.\n");
  }
  if (CHECK_BIT(cfsr, DACCVIOL))
  {
    printf(" - Load or store at a location that does not permit the operation.\n");
  }
  if (CHECK_BIT(cfsr, IACCVIOL))
  {
    printf(" - Instruction fetch from a location that does not permit execution.\n");
  }
  printf("Bus fault status:\n");
  if (CHECK_BIT(cfsr, BFARVALID))
  {
    printf(" - BFAR holds a valid address.\n");
  }
  else
  {
    printf(" - BFAR holds an invalid address.\n");
  }
  if (CHECK_BIT(cfsr, LSPERR))
  {
    printf(" - Fault occurred during floating-point lazy state preservation.\n");
  }
  if (CHECK_BIT(cfsr, STKERR))
  {
    printf(" - Stacking has caused a Bus fault.\n");
  }
  if (CHECK_BIT(cfsr, UNSTKERR))
  {
    printf(" - Unstacking has caused a Bus fault.\n");
  }
  if (CHECK_BIT(cfsr, IMPRECISERR))
  {
    printf(" - Data bus error has occurred, but the return address in the stack is not related to the fault.\n");
  }
  if (CHECK_BIT(cfsr, PRECISERR))
  {
    printf(" - Data bus error has occurred, and the return address points to the instruction that caused the fault.\n");
  }
  if (CHECK_BIT(cfsr, IBUSERR))
  {
    printf(" - Instruction bus error.\n");
  }
  printf("Usage fault status:\n");
  if (CHECK_BIT(cfsr, DIVBYZERO))
  {
    printf(" - The processor has executed an SDIV or UDIV instruction with a divisor of 0.\n");
  }
  if (CHECK_BIT(cfsr, UNALIGNED))
  {
    printf(" - The processor has made an unaligned memory access.\n");
  }
  if (CHECK_BIT(cfsr, NOCP))
  {
    printf(" - Attempted to access a coprocessor.\n");
  }
  if (CHECK_BIT(cfsr, INVPC))
  {
    printf(" - Illegal attempt to load of EXC_RETURN to the PC.\n");
  }
  if (CHECK_BIT(cfsr, INVSTATE))
  {
    printf(" - Attempted to execute an instruction that makes illegal use of the EPSR.\n");
  }
  if (CHECK_BIT(cfsr, INVSTATE))
  {
    printf(" - The processor has attempted to execute an undefined instruction.\n");
  }

  /* Breakpoint. */
  // __asm volatile("BKPT #0");
    Error_Handler();

  /* Infinite loop to stop the execution. */
  while(1);
}
