SUFFIX  = .c
CC      = clang
CXX     = clang++
CFLAGS  = -g -O0 -Xclang -load -Xclang
IRFLAGS = -S -emit-llvm
ifndef $(FILE)
FILE    = ../../build/st_free/libStructFreeMod.so
endif
LIBS    =
LOG_DIR = log
SRC_DIR = src
OBJ_DIR = ir
TGT_DIR = bin
SOURCES = $(wildcard $(SRC_DIR)/*$(SUFFIX))
OBJECTS = $(notdir $(SOURCES:$(SUFFIX)=.o))
TARGETS = $(notdir $(basename $(SOURCES)))
IRS     = $(notdir $(SOURCES:$(SUFFIX)=.ll))

all: $(TARGETS) $(IRS)

path: $(TARGETS)

ir: $(IRS)

$(TARGETS): $(SOURCES)
	@if [ ! -d $(TGT_DIR) ]; then \
		echo "Directory $(TGT_DIR) does not exist. Creating...";\
		mkdir -p $(TGT_DIR);\
		fi
	@if [ ! -d $(LOG_DIR) ]; then \
		echo "Directory $(LOG_DIR) does not exist. Creating...";\
		mkdir -p $(LOG_DIR);\
		fi
	@echo '================================='
	@echo 'compiling' $@ 'with file:' $(FILE)
	head -n 1 $(SRC_DIR)/$@$(SUFFIX)
	$(CC) $(CFLAGS) $(FILE) -o $(TGT_DIR)/$@ $(SRC_DIR)/$@$(SUFFIX)

$(IRS): $(SOURCES)
	@if [ ! -d $(OBJ_DIR) ] ; then \
		echo "Directory $(OBJ_DIR) does not exist. Creating...";\
		mkdir -p $(OBJ_DIR);\
		fi
	@echo 'creating LLVM-IR:' $@
	$(CC) $(IRFLAGS) -o $(OBJ_DIR)/$@ $(SRC_DIR)/$(notdir $(@:.ll=$(SUFFIX)))

clean:
	$(RM) -r $(TGT_DIR)/* $(OBJ_DIR)/* $(LOG_DIR)/*.log

clean_log:
	$(RM) -r $(LOG_DIR)/*.log
