Nested SVM Availability Probe
=============================

This test probes whether AMD SVM is visible to an hvm64 or qemu64 guest.  It
checks the collected CPUID SVM feature, enables EFER.SVME using the shared
nested-SVM setup helper, and confirms that EFER.SVME is set before reporting
success.
