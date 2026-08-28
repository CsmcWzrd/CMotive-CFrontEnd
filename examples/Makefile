# CMotive Lang-Examples Makefile using the native C CMotive frontend.
# Override CMOTIVE_ROOT, CMOTIVE, or CMOTIVEPP if your layout differs.

SHELL := /bin/sh
CMOTIVE_ROOT ?= ../cmotive-source
CMOTIVE ?= $(CMOTIVE_ROOT)/build/bin/cmotive
CMOTIVEPP ?= $(CMOTIVE_ROOT)/build/bin/cmotivepp
TIMEOUT ?= 5
EXRUN := CMOTIVE_ROOT='$(CMOTIVE_ROOT)' CMOTIVE='$(CMOTIVE)' CMOTIVEPP='$(CMOTIVEPP)' TIMEOUT='$(TIMEOUT)' sh tools/example_build.sh

.PHONY: all compile objects run check preprocess manifest clean list help debug-symbols vs-validate

all: compile

help:
	@printf '%s\n' 'CMotive Lang-Examples targets:'
	@printf '%s\n' '  make compile     Compile all manifest examples to build/bin'
	@printf '%s\n' '  make objects     Compile all manifest examples with -c to build/obj'
	@printf '%s\n' '  make run         Compile and execute all manifest examples'
	@printf '%s\n' '  make check       Manifest + run verification'
	@printf '%s\n' '  make preprocess  Run cmotivepp over all examples into build/pp'
	@printf '%s\n' '  make clean       Remove Lang-Examples build outputs'
	@printf '%s\n' '  make debug-symbols Build example 150 with -g3 -O2 and verify .syms output'
	@printf '%s\n' '  make list        List manifest examples'
	@printf '%s\n' '  make vs-validate Validate the VS2022 native-example project metadata'

manifest:
	@$(EXRUN) manifest

compile:
	@$(EXRUN) compile

objects:
	@$(EXRUN) objects

preprocess:
	@$(EXRUN) preprocess

run:
	@$(EXRUN) run

check: manifest run

list:
	@$(EXRUN) list

debug-symbols:
	@$(EXRUN) debug-symbols

vs-validate:
	@test -f vs2022/CMotive.LangExamples.Packages.vcxproj
	@echo 'VS2022 project metadata present; native CMotive compiler is expected under $(CMOTIVE_ROOT)/build/bin.'

clean:
	@$(EXRUN) clean
