STAGE_ONE =./StageOne
STAGE_TWO =./StageTwo

.PHONY: StageOne StageTwo clean all
all: StageOne StageTwo

StageOne:
	@echo === Testing StageOne ===
	$(MAKE) -C $(STAGE_ONE)

StageTwo:
	@echo === Testing StageTwo ===
	$(MAKE) -C $(STAGE_TWO)

clean:
	$(MAKE) -C $(STAGE_ONE) clean
	$(MAKE) -C $(STAGE_TWO) clean

