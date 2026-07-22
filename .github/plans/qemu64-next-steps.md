# QEMU64 Nested-SVM Handoff

This plan captures the current `qemu64` milestone and the information needed to
continue the work in a new chat session on a host that can run QEMU with KVM.

## Current Status

- Commit `6a4a9799e27a` (`x86: add qemu64 bare-metal smoke environment`) adds
  the first `qemu64` environment.
- `qemu64` is explicitly opt-in with `QEMU-TEST-ENVS`; it is intentionally not
  part of `ALL_ENVIRONMENTS`.
- `tests/example` is the only test currently opted in.
- `tests/example/test-qemu64-example` builds as a native ELF64 image and boots
  directly with `qemu-system-x86_64 -kernel`.
- The QEMU path prints through COM1 serial and exits through `isa-debug-exit`,
  without Xen hypercall setup, Xen shared info, PV console, or xenbus setup.
- No `xl` configuration files are generated for qemu-only environments.

## Verified Commands

The following checks passed on the original implementation host:

```sh
make TESTS=tests/example
make
rm -f tests/example/test-qemu64-example
make TESTS=tests/example USE_MAKE=1
git --no-pager diff --check
```

The QEMU smoke test also passed:

```sh
qemu-system-x86_64 \
  -kernel tests/example/test-qemu64-example \
  -display none \
  -serial stdio \
  -no-reboot \
  -device isa-debug-exit,iobase=0xf4,iosize=0x04
```

The same baseline can also be run through the repo helper:

```sh
.github/scripts/qemu64-kvm-smoke.sh --tcg
```

On the KVM continuation host, use the default KVM mode:

```sh
.github/scripts/qemu64-kvm-smoke.sh
```

Expected console output:

```text
--- Xen Test Framework ---
Environment: QEMU 64bit (Long mode 4 levels)
Hello World
Test result: SUCCESS
```

`pre-commit` was not installed in the original sandbox. Prefer `prek`, which is
fully compatible with `pre-commit`, for the final validation command:

```sh
SKIP=git-diff prek run -av
```

If `prek` is unavailable on another host, use the compatible fallback:

```sh
SKIP=git-diff pre-commit run -av
```

`make runner-selftest` was also tried and failed in an apparently unrelated
existing expectation: `xtf.runner.selftest.RunnerCliTests.test_list_prints_selected_test_instances`
expects `runner.main()` to return `None` for `--list`, but the current runner
returns `0`. The qemu64 work did not modify `xtf/runner/cli.py` or
`xtf/runner/selftest.py`.

## Design Notes

- `qemu64` reuses the HVM low-level object set and sets both `CONFIG_HVM` and
  `CONFIG_QEMU`.
- `CONFIG_QEMU` is the discriminator for skipping Xen runtime setup in
  `arch_setup()`.
- `qemu64` must stay native ELF64. QEMU rejected the Xen `hvm64` converted
  `elf32-x86-64` style image with `Cannot load x86-64 image, give a 32bit one`.
- The existing Xen PVH ELF note in `arch/x86/hvm/head.S` is sufficient for
  QEMU's `-kernel` loader when the image is left as ELF64.
- QEMU exit uses `outl(reason, 0xf4)`. With `isa-debug-exit`, the process exit
  status is derived from the written value, so the console result line is the
  most reliable success signal for now.
- The current milestone deliberately does not add an `xtf-runner` QEMU mode.
  Use direct `qemu-system-x86_64 -kernel ...` commands until a runner workflow
  is requested.

## Host Requirements For AMD SVM Work

Continue nested-SVM work on a host with AMD virtualization and usable KVM.
Before changing any nested-SVM test, verify:

```sh
command -v qemu-system-x86_64
test -r /dev/kvm && test -w /dev/kvm
.github/scripts/qemu64-kvm-smoke.sh
```

Also verify that SVM is exposed to the guest before enabling nested-SVM tests.
Pure TCG should not be treated as adequate validation for AMD SVM instructions
or nested virtualization semantics.

## Next Implementation Steps

1. Re-run the existing `qemu64` example smoke test with KVM enabled on the new
   host.
2. Add the smallest SVM availability probe for `qemu64`, or opt in one nested
   SVM test only after guest-visible SVM is confirmed.
3. Candidate first nested test: `tests/nested-svm-run`, because the test body is
   mostly CPU/MSR/VMCB work and should not need Xen services in principle.
4. Keep opt-in explicit by adding `QEMU-TEST-ENVS := $(QEMU_ENVIRONMENTS)` only
   to tests that are known to avoid Xen-only APIs.
5. Validate with the QEMU smoke, full `make`, and finally
  `SKIP=git-diff prek run -av`.
