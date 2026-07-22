#!/bin/sh

set -eu

usage()
{
    cat <<EOF
Usage: $0 [--kvm|--tcg] [KERNEL]

Boot an XTF qemu64 test with qemu-system-x86_64 and require the console to
contain "Test result: SUCCESS".  The default kernel is:

  tests/example/test-qemu64-example

Options:
  --kvm  Run with -accel kvm -cpu host.  This is the default.
  --tcg  Run with -accel tcg.  Useful only for the baseline smoke test; do not
         treat TCG as nested-SVM validation.
EOF
}

mode=kvm
kernel=tests/example/test-qemu64-example

while [ "$#" -gt 0 ]; do
    case "$1" in
    --help|-h)
        usage
        exit 0
        ;;
    --kvm)
        mode=kvm
        ;;
    --tcg)
        mode=tcg
        ;;
    --*)
        usage >&2
        exit 2
        ;;
    *)
        kernel=$1
        ;;
    esac
    shift
done

repo=$(git rev-parse --show-toplevel 2>/dev/null || pwd)
cd "$repo"

if ! command -v qemu-system-x86_64 >/dev/null 2>&1; then
    echo "qemu-system-x86_64 not found" >&2
    exit 1
fi

if [ ! -r "$kernel" ]; then
    echo "Kernel '$kernel' not found; build it first with 'make TESTS=tests/example'" >&2
    exit 1
fi

accel_args=
case "$mode" in
kvm)
    if [ ! -r /dev/kvm ] || [ ! -w /dev/kvm ]; then
        echo "/dev/kvm is not readable and writable by this user" >&2
        exit 1
    fi

    if [ -r /proc/cpuinfo ] && ! grep -qw svm /proc/cpuinfo; then
        echo "warning: host /proc/cpuinfo does not advertise svm" >&2
    fi

    accel_args="-accel kvm -cpu host"
    ;;
tcg)
    accel_args="-accel tcg"
    ;;
*)
    usage >&2
    exit 2
    ;;
esac

log=$(mktemp)
trap 'rm -f "$log"' EXIT

set +e
# shellcheck disable=SC2086
qemu-system-x86_64 $accel_args \
    -kernel "$kernel" \
    -display none \
    -serial stdio \
    -no-reboot \
    -device isa-debug-exit,iobase=0xf4,iosize=0x04 \
    >"$log" 2>&1
rc=$?
set -e

cat "$log"

if ! grep -q "Test result: SUCCESS" "$log"; then
    echo "qemu64 smoke failed; qemu exit status was $rc" >&2
    exit 1
fi

echo "qemu64 smoke passed; qemu exit status was $rc"
