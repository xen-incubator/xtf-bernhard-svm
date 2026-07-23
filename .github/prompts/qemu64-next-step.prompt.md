---
name: 'Continue XTF qemu64 nested-SVM work'
description: 'Continue the qemu64 direct-QEMU boot plan on a KVM-capable host and take the next nested-SVM implementation step.'
agent: 'XTF QEMU64 Continuation'
tools: [read, search, edit, execute]
---

Continue the XTF qemu64 nested-SVM enablement work on this KVM-capable host.

First read:

- [.github/plans/qemu64-next-steps.md](../plans/qemu64-next-steps.md)
- [.github/skills/xtf-qemu64-handoff/SKILL.md](../skills/xtf-qemu64-handoff/SKILL.md)
- [.github/agents/xtf-qemu64.agent.md](../agents/xtf-qemu64.agent.md)

Use the qemu64 handoff guidance. The existing baseline is:

- `qemu64` is opt-in only via `QEMU-TEST-ENVS`.
- `tests/example/test-qemu64-example` boots directly with
  `qemu-system-x86_64 -kernel`.
- QEMU output goes to COM1 serial.
- QEMU exits via `isa-debug-exit`.
- `qemu64` must remain native ELF64.
- Do not add `qemu64` to `ALL_ENVIRONMENTS`.
- Do not add an `xtf-runner` QEMU mode unless explicitly needed.

This host has nested KVM available. Verify the host and baseline first:

```sh
command -v qemu-system-x86_64
test -r /dev/kvm && test -w /dev/kvm
.github/scripts/qemu64-kvm-smoke.sh
```

Then determine the smallest next implementation step toward running nested-SVM
tests under `qemu64`. Prefer adding a minimal `qemu64` SVM availability probe
before opting in a larger nested-SVM test. If guest-visible SVM is confirmed,
consider opt-in work for `tests/nested-svm-run`, preserving existing Xen
behavior.

Keep `qemu64` support explicit and narrowly scoped. Validate with:

```sh
.github/scripts/qemu64-kvm-smoke.sh
make
SKIP=git-diff prek run -av
```

Commit changes with focused commit messages.
