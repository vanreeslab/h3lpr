# Makefile H3LPR
#------------------------------------------------------------------------------
# useful links: 
# - automatic vars: https://www.gnu.org/software/make/manual/html_node/Automatic-Variables.html
# - file names: https://www.gnu.org/software/make/manual/html_node/File-Name-Functions.html
#------------------------------------------------------------------------------
include makefun.mak

################################################################################
# ARCH DEPENDENT VARIABLES
ARCH_FILE ?= make_arch/make.default

#only include the ARCH_FILE when we do not clean or destroy
ifneq ($(MAKECMDGOALS),clean)
include $(ARCH_FILE)
endif

################################################################################
# FROM HERE, DO NOT CHANGE
UNAME := $(shell uname)

#-------------------------------------------------------------------------------
PREFIX ?= ./
NAME := h3lpr
# library naming
TARGET := $(NAME)
# git commit
GIT_COMMIT ?= $(shell git describe --always --dirty)

#-------------------------------------------------------------------------------
# get a list of all the source directories + the main one
SRC_DIR := src $(shell find src/** -type d)
TEST_DIR := test
OBJ_DIR := build

#-------------------------------------------------------------------------------
# the sources/headers are listed without the folder, vpath will find them
SRC := $(foreach dir,$(SRC_DIR),$(notdir $(wildcard $(dir)/*.cpp)))
# HEAD := $(foreach dir,$(SRC_DIR),$(notdir $(wildcard $(dir)/*.hpp)))
HEAD := $(foreach dir,$(SRC_DIR),$(wildcard $(dir)/*.hpp))

# find the test sources, mandatory all in the same folder!
TSRC := $(foreach dir,$(SRC_DIR),$(notdir $(wildcard $(TEST_DIR)/$(dir)/*.cpp)))
# THEAD := $(foreach dir,$(SRC_DIR),$(notdir $(wildcard $(TEST_DIR)/$(dir)/*.hpp)))
THEAD := $(foreach dir,$(SRC_DIR),$(wildcard $(TEST_DIR)/$(dir)/*.hpp))

## generate object list
OBJ := $(SRC:%.cpp=$(OBJ_DIR)/%.o)
DEP := $(SRC:%.cpp=$(OBJ_DIR)/%.d)
CDB := $(SRC:%.cpp=$(OBJ_DIR)/%.o.json)
TIDY := $(SRC:%.cpp=$(OBJ_DIR)/%.tidy)

TOBJ := $(TSRC:%.cpp=$(TEST_DIR)/$(OBJ_DIR)/%.o)
TDEP := $(TSRC:%.cpp=$(TEST_DIR)/$(OBJ_DIR)/%.d)
TCDB := $(TSRC:%.cpp=$(TEST_DIR)/$(OBJ_DIR)/%.o.json)

#-------------------------------------------------------------------------------
# add the folders to the includes and to the vpath

## add the source dirs to the includes flags
INC := $(foreach dir,$(SRC_DIR),-I$(dir))
TINC := $(foreach dir,$(TEST_DIR)/$(SRC_DIR),-I$(dir))

## add them to the VPATH as well (https://www.gnu.org/software/make/manual/html_node/Selective-Search.html)
vpath %.hpp $(SRC_DIR) $(foreach dir,$(SRC_DIR),$(TEST_DIR)/$(dir))
vpath %.cpp $(SRC_DIR) $(foreach dir,$(SRC_DIR),$(TEST_DIR)/$(dir))

#-------------------------------------------------------------------------------
# LIBRARIES
#---- GTEST
GTEST_INC ?= /usr/include
GTEST_LIB ?= /usr/lib
GTEST_LIBNAME ?= -lgtest

################################################################################
# mandatory flags
M_FLAGS := -std=c++17 -fPIC -DGIT_COMMIT=\"$(GIT_COMMIT)\"

#-------------------------------------------------------------------------------
# compile + dependence + json file
$(OBJ_DIR)/%.o : %.cpp
ifeq ($(shell $(CXX) -v 2>&1 | grep -c "clang"), 1)
	$(CXX) $(CXXFLAGS) $(OPTS) $(INC) $(DEF) $(M_FLAGS) -MMD -MF $(OBJ_DIR)/$*.d -MJ $(OBJ_DIR)/$*.o.json -c $< -o $@
else
	$(CXX) $(CXXFLAGS) $(OPTS) $(INC) $(DEF) $(M_FLAGS) -MMD -c $< -o $@
endif

# json only
$(OBJ_DIR)/%.o.json :  %.cpp
	$(CXX) $(CXXFLAGS) $(OPTS) $(INC) $(DEF) $(M_FLAGS) -MJ $@ -E $< -o $(OBJ_DIR)/$*.ii

#-------------------------------------------------------------------------------
# tests
# compile + dependence + json
$(TEST_DIR)/$(OBJ_DIR)/%.o : %.cpp
ifeq ($(shell $(CXX) -v 2>&1 | grep -c "clang"), 1)
	$(CXX) $(CXXFLAGS) $(OPTS) $(TINC) $(INC) -I$(GTEST_INC) $(DEF) $(M_FLAGS) -MMD -MF $(OBJ_DIR)/$*.d -MJ $(OBJ_DIR)/$*.o.json -c $< -o $@
else
	$(CXX) $(CXXFLAGS) $(OPTS) $(TINC) $(INC) -I$(GTEST_INC) $(DEF) $(M_FLAGS) -MMD -c $< -o $@
endif

# json only
$(TEST_DIR)/$(OBJ_DIR)/%.o.json : %.cpp
	$(CXX) $(CXXFLAGS) $(OPTS) $(TINC) $(INC) -I$(GTEST_INC) $(DEF) $(M_FLAGS) -MJ $@ -E $< -o $(TEST_DIR)/$(OBJ_DIR)/$*.ii

# clang-tidy files, define the MPI_INC which is only for this target
$(OBJ_DIR)/%.tidy : %.cpp
	clang-tidy $< --format-style=.clang-format --checks=all*

################################################################################
.PHONY: default
default: lib_dynamic lib_static

.PHONY: all
all: lib_dynamic lib_static compdb

#-------------------------------------------------------------------------------
.PHONY: lib_dynamic
lib_dynamic: $(TARGET).so $(TARGET).dylib

.PHONY: lib_static
lib_static: $(TARGET).a

# the main target
$(TARGET).so: $(OBJ)
	$(CXX) -shared $(LDFLAGS) $^ $(LIB) -o $@

# generate the .dylib only on darwin systems
$(TARGET).dylib: $(OBJ)
ifeq ($(UNAME), Darwin)
	$(CXX) -dynamiclib $(LDFLAGS) -install_name $(PREFIX)/lib/lib$@ $^ $(LIB) -o $@
endif

$(TARGET).a: $(OBJ)
	ar rvs $@ $^

#-------------------------------------------------------------------------------
# clang stuffs
.PHONY: tidy
tidy: $(TIDY)

# for the sed commande, see https://sarcasm.github.io/notes/dev/compilation-database.html#clang and https://sed.js.org
.PHONY: compdb
compdb: $(CDB)
	sed -e '1s/^/[\n/' -e '$$s/,$$/\n]/' $^ > compile_commands.json

# build the full compilation data-base, need to know the test libs!!
.PHONY: compdb_full
compdb_full: $(CDB) $(TCDB)
	sed -e '1s/^/[\n/' -e '$$s/,$$/\n]/' $^ > compile_commands.json

#-------------------------------------------------------------------------------
.PHONY: test 
test: $(TOBJ) $(OBJ)
	$(CXX) $(LDFLAGS) $^ -o $(TARGET)_$@ $(LIB) -L$(GTEST_LIB) $(GTEST_LIBNAME) -Wl,-rpath,$(GTEST_LIB)

#-------------------------------------------------------------------------------
.PHONY: install
install: info lib_dynamic lib_static | install_dir
	$(call copy_list,$(HEAD),$(PREFIX)/include/${NAME})
	$(call mv_list,$(TARGET).a,$(PREFIX)/lib/lib$(TARGET).a)
	$(call mv_list,$(TARGET).so,$(PREFIX)/lib/lib$(TARGET).so)
ifeq ($(UNAME), Darwin)
	$(call mv_list,$(TARGET).dylib,$(PREFIX)/lib/lib$(TARGET).dylib)
endif

.PHONY: install_dir
install_dir:
	@mkdir -p $(PREFIX)/lib
	@mkdir -p $(PREFIX)/include/$(NAME)


#-------------------------------------------------------------------------------
#clean
.PHONY: clean
clean:
	@rm -rf $(TARGET).so
	@rm -rf $(TARGET).a
	@rm -rf $(TARGET)_test
	@rm -rf $(OBJ_DIR)/*
	@rm -rf $(PREFIX)/lib/$(TARGET)*
	@rm -rf $(PREFIX)/include/$(NAME)/*
	@rm -rf $(TEST_DIR)/$(OBJ_DIR)/*.o
	
#-------------------------------------------------------------------------------
.EXPORT_ALL_VARIABLES:

.PHONY: info
info: 
	@$(MAKE) --file=info.mak
                                

-include $(DEP) $(TDEP)

# mkdir the needed dirs
$(shell   mkdir -p $(OBJ_DIR) $(TEST_DIR)/$(OBJ_DIR))
