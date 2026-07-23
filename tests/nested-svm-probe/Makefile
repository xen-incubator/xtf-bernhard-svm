include $(ROOT)/build/common.mk

NAME      := nested-svm-probe
CATEGORY  := nested-svm
TEST-ENVS := $(SVM_ENVIRONMENTS)
QEMU-TEST-ENVS := $(QEMU_ENVIRONMENTS)

obj-perenv += main.o
obj-perenv += $(ROOT)/common/nested-svm/setup-l2.o

include $(ROOT)/build/gen.mk
