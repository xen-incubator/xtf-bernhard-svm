/**
 * @file tests/nested-svm-clgi-stgi/main.c
 * @ref test-nested-svm-clgi-stgi
 *
 * @page test-nested-svm-clgi-stgi nested-svm-clgi-stgi
 *
 * Smoke-test of AMD SVM nested virtualisation:
 *
 * An L1 guest:
 * 1. enables SVM,
 * 2. builds a minimal L2 VMCB that re-uses L1's address space, and
 * 3. uses CLGI/STGI to manage interrupts in L2.
 *
 * L2:
 * 1. increments %rax
 * 2. signals completion with HLT, which causes a #VMEXIT back to L1.
 *
 * @see tests/nested-svm-clgi-stgi/main.c
 */
#include <nested-svm/setup-l2.h>

const char test_title[] = "Nested SVM CLGI/STGI Smoke Test";

/**
 * L2 Interrupt Service Routine for Vector 0x30.
 * It is used to prove the interrupt was actually taken.
 */
static void __used l2_isr_0x30(void)
{
    /* Decrement rax before exit to L1 to indicate interrupt was taken */
    asm volatile ("dec %rax;hlt");
}

static bool l2_isr_0x30_called(void)
{
    return l2_vmcb.rax == 1; /* L2 should have decremented %rax from 2 to 1 */
}

/**
 * Run a minimal L2 payload. If an interrupt is pending,
 * it will be taken after the instruction following STI.
 */
static void __used l2_entry(void)
{
    /* STI to allow interrupts, then increment %rax */
    asm volatile ("sti\n"
                  /* Dummy insn ensures STI takes effect before inc %rax */
                  "nop\n" 
                  "inc %rax\n"
                  "hlt");
}

static bool run_l2(const char *file, int line)
{
    l2_vmcb.rip = _u(l2_entry);
    l2_vmcb.rax = 2; /* Starting value for L2 to inc/decrement before HLT */
    intr_ctrl->irq_is_pending = 1; /* vIRQ request enable bit */
    print_v_intr_ctrl(file, line, &l2_vmcb, "pre-VMRUN ");
    asm volatile("mov %0, %%rax\n"
                 "vmload %%rax\n"
                 "vmrun %%rax\n"
                 "vmsave %%rax\n"
                 :
                 : "r" (_u(&l2_vmcb))
                 : "%rax", "memory");
    print_v_intr_ctrl(file, line, &l2_vmcb, "post-VMRUN");

    if ( l2_vmcb.exitcode != VMEXIT_HLT )
    {
        xtf_failure("%s:%d: unexpected L2 exit: 0x%lx (%s)\n", file, line,
                    l2_vmcb.exitcode, vmexit_reason(l2_vmcb.exitcode));
        return false;
    }
    return true;
}

#define FAIL(fmt, ...) \
    xtf_failure("%s:%d: " fmt, __FILE__, __LINE__, ##__VA_ARGS__)

/**
 * Execute the nested-SVM CLGI/STGI smoke test.
 *
 * L1 enables SVM, prepares a minimal L2 VMCB, enters L2 once with VMRUN
 * and verifies that L2 reports success before exiting with HLT.
 */
void test_main(void)
{
    /* Enable SVM, arm the host-save area and build the L2 VMCB. */
    if (!svm_l1_enable_svm())
        return;
    svm_l2_build_vmcb(&l2_vmcb, NULL);

    /* Set up L2's IDTR and install the interrupt gate used by the test. */
    setup_l2_idt(&l2_vmcb, 0x30, l2_isr_0x30);
    intr_ctrl->vector = 0x30; /* Use vector 0x30 when injecting interrupts */

    /* Set the L2 entry point to this test's l2_entry function. */
    l2_vmcb.rip = _u(l2_entry);

    /* Inject a pending virtual hardware interrupt (Vector 0x30, Priority 2) */
    intr_ctrl->irq_is_pending = 1; /* vIRQ request enable bit */

    /* Starting value for L2 to inc/decrement before HLT */
    if (!run_l2(__FILE__, __LINE__))
        return;
    if (!l2_isr_0x30_called())
        return FAIL("L2 interrupt handler was not called as expected\n");

    intr_ctrl->vGIF_is_enabled = 1; /* Enable vGIF */
    intr_ctrl->vGIF = 0; /* disable interrupts */

    /* TODO: Test running this in L2 and check vGIF behavior and state */
    // asm volatile("stgi\n");

    if (!run_l2(__FILE__, __LINE__))
        return;
    if (l2_isr_0x30_called())
        return FAIL("L2 isr  called when vGIF:0 should have blocked it\n");

    intr_ctrl->vGIF = 1; /* enable interrupts */
    if (!run_l2(__FILE__, __LINE__))
        return;
    if (!l2_isr_0x30_called())
        return FAIL("L2 isr was not called as expected when vGIF:1\n");

    xtf_success(NULL);
}
