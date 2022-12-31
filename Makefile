CXX := clang++
CXXOPT := -O3

CXXWARNINGS := -Wall -Wextra -pedantic -Wimplicit-fallthrough
CXXWERROR := -Werror
CXXWERROR += -Wno-error=unused-parameter
CXXWERROR += -Wno-error=unused-private-field
CXXWERROR += -Wno-error=unused-variable
CXXWERROR += -Wno-error=unused-function
# CXXWERROR += -Wconversion
CXXWERROR += -Wno-error=sign-conversion
CXXWERROR += -Wno-unused-but-set-variable
CXXFLAGS := -g -std=c++20 -Isrc -fno-omit-frame-pointer
CXXFLAGS += $(CXXOPT)
CXXFLAGS += $(CXXWARNINGS)
CXXFLAGS += $(CXXWERROR)
CXXFLAGS += -march=native

BUILDDIR := build
TESTDIR := tests
OBJDIR := $(BUILDDIR)/objs-$(CXX)

PRECIOUS_TARGETS += $(BUILDDIR)

SRCS := $(shell find src/algos -iname "*.cpp")
OBJS := $(SRCS:%.cpp=$(OBJDIR)/%.o)
TARGS := $(SRCS:%.cpp=$(BUILDDIR)/%)
DEPS_HEADER := $(shell find src -maxdepth 1 -iname "*.h")
DEPS_SOURCE := $(shell find src -maxdepth 1 -iname "*.cpp")

all: $(TARGS)

$(OBJDIR)/%.o: %.cpp Makefile
	mkdir -p $(shell dirname $@)
	$(COMPILE.cpp) -MMD $(OUTPUT_OPTION) $< 

PRECIOUS_TARGETS += $(OBJDIR)/%.o 

$(BUILDDIR)/%: $(OBJDIR)/%.o $(DEPS_SOURCE) Makefile
	mkdir -p $(shell dirname $@)
	$(LINK.cpp)  -MMD $(LDLIBS) $(OUTPUT_OPTION) $(filter-out Makefile,$^)

clean: Makefile
	rm -fr $(BUILDDIR)

clean-tests:
	rm -fr $(TESTDIR)/

tests:
	python3 generate.py

format:
	clang-format -i $(SRCS)
