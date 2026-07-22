---
name: xtf-qemu64-handoff
description: 'Use when continuing XTF qemu64, bare-metal QEMU, KVM, AMD SVM, or nested-SVM enablement work in this repository.'
argument-hint: 'Continue qemu64 or nested-SVM work'
---

# XTF QEMU64 Handoff

Use this skill for work that extends the `qemu64` environment, moves XTF tests
from Xen-only execution toward direct QEMU execution, or prepares nested-SVM
tests for a KVM-capable AMD host.

## Repository Rules

- Build from the repository root with `make` for final validation.
- Do not rely only on subset builds before finishing a change.
- Prefer `prek`, which is fully compatible with `pre-commit`, for the final
  validation command:

  ```sh
  SKIP=git-diff prek run -av
  ```

- If `prek` is unavailable, fall back to
  `SKIP=git-diff pre-commit run -av`.
- Preserve existing Xen behavior unless the task explicitly asks to change it.

## QEMU64 Model

- `qemu64` is opt-in only. Do not add it to `ALL_ENVIRONMENTS`.
- Tests opt in with:

  ```make
  QEMU-TEST-ENVS := $(QEMU_ENVIRONMENTS)
  ```

- Only opt in tests that avoid Xen-only runtime services or have been adapted
  to the `CONFIG_QEMU` path.
- QEMU binaries should not get generated `xl` configuration files.
- Keep `qemu64` as native ELF64. Do not convert it through the Xen `hvm64`
  `elf32-x86-64` objcopy path.
- The current QEMU runtime uses COM1 serial output and `isa-debug-exit` at port
  `0xf4`.

## Baseline Smoke Test

Use the example test to check that the direct QEMU path still works:

```sh
qemu-system-x86_64 \
  -kernel tests/example/test-qemu64-example \
  -display none \
  -serial stdio \
  -no-reboot \
  -device isa-debug-exit,iobase=0xf4,iosize=0x04
```

Expected output includes:

```text
--- Xen Test Framework ---
Environment: QEMU 64bit (Long mode 4 levels)
Hello World
Test result: SUCCESS
```

The QEMU process status may be nonzero because of `isa-debug-exit`; check the
console result line.

## Nested-SVM Continuation

Before adding AMD SVM instructions or opting in nested-SVM tests, move to a host
with usable KVM and AMD SVM exposed to guests.

Check at least:

```sh
command -v qemu-system-x86_64
test -r /dev/kvm && test -w /dev/kvm
qemu-system-x86_64 -accel kvm -cpu host \
  -kernel tests/example/test-qemu64-example \
  -display none \
  -serial stdio \
  -no-reboot \
  -device isa-debug-exit,iobase=0xf4,iosize=0x04
```

Treat pure TCG as insufficient for nested-SVM semantics. If guest-visible SVM is
not confirmed, do not opt in `tests/nested-svm-*` for `qemu64` yet.

## Useful Handoff Reference

Read `.github/plans/qemu64-next-steps.md` before continuing this line of work.
