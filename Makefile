SERIAL := $(shell system_profiler SPHardwareDataType | awk '/Serial/ {print $$4}')
ifeq ($(SERIAL),C02TM0RNHF1R)
	PASS_DIR = build13
else
	PASS_DIR = build
	# PASS_DIR = build_home
endif
TEST_DIR = test_set
FILE     = ../$(PASS_DIR)/st_free/libStructFreeMod.so

all:
	@echo === PASS_DIR is $(PASS_DIR) ===
	@echo === creating .so file       ===
	$(MAKE) -C $(PASS_DIR)
	@echo === cleaning test directory ===
	$(MAKE) -C $(TEST_DIR) clean
	@echo === creating test sets      ===
	$(MAKE) -C $(TEST_DIR) FILE=$(FILE)
