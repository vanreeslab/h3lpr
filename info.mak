.NOTPARALLEL: logo
#-------------------------------------------------------------------------------
.PHONY: logo info
info: logo
	$(info install dir = $(PREFIX)/lib and $(PREFIX)/include )
	$(info compiler = $(shell $(CXX) --version))
	$(info compil. options = $(OPTS))
	$(info compil. flags = $(CXXFLAGS) $(OPTS) $(INC) $(DEF) -fPIC -MMD)
	$(info linker flags = -shared $(LDFLAGS))
	$(info using arch file = $(ARCH_FILE) )
	$(info ---------------------------------)
	$(info SOURCES)
	$(info - SRC  = $(SRC))
	$(info - HEAD = $(HEAD))
	$(info - HEAD = $(call to_list,$(HEAD)))
	$(info - OBJ  = $(OBJ))
	$(info - DEP  = $(DEP))
	$(info ---------------------------------)
	$(info TESTING:)
	$(info - TEST_DIR = $(TEST_DIR))
	$(info - TEST_DIR = $(TEST_DIR)/$(OBJ_DIR))
	$(info - test SRC = $(TSRC))
	$(info - test OBJ = $(TOBJ))
	$(info - test DEP = $(TDEP))
	$(info ---------------------------------)



logo:
	$(info )
	$(info ██╗  ██╗██████╗ ██╗     ██████╗ ██████╗  )
	$(info ██║  ██║╚════██╗██║     ██╔══██╗██╔══██╗ )
	$(info ███████║ █████╔╝██║     ██████╔╝██████╔╝ )
	$(info ██╔══██║ ╚═══██╗██║     ██╔═══╝ ██╔══██╗ )
	$(info ██║  ██║██████╔╝███████╗██║     ██║  ██║ )
	$(info ╚═╝  ╚═╝╚═════╝ ╚══════╝╚═╝     ╚═╝  ╚═╝ )
	$(info )
	$(info ----------------------------------------------- )
                                
