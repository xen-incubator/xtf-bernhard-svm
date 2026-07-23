/**
 * @file tests/nested-svm-probe/main.c
 * @ref test-nested-svm-probe
 * @page test-nested-svm-probe Nested SVM Availability Probe
 *
 * Probes whether AMD SVM is visible to the guest and whether EFER.SVME can be
 * enabled.  This is the smallest qemu64 nested-SVM check before opting in a
 * full VMRUN test.
 *
 * @see tests/nested-svm-probe/main.c
 */
#include <nested-svm/setup-l2.h>

const char test_title[] = "Nested SVM Availability Probe";

void test_main(void)
{
    if ( !svm_l1_enable_svm() )
        return;

    print_efer("L1", rdmsr(MSR_EFER));

    if ( !(rdmsr(MSR_EFER) & EFER_SVME) )
        return xtf_failure("EFER.SVME is clear after enabling SVM\n");

    xtf_success(NULL);
}

/*
 * Local variables:
 * mode: C
 * c-file-style: "BSD"
 * c-basic-offset: 4
 * tab-width: 4
 * indent-tabs-mode: nil
 * End:
 */
