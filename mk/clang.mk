# CROSS_COMPILE_$(sm) is derived from CROSS_COMPILE/CROSS_COMPILE32/
# CROSS_COMPILE64 and indicates the target that should be passed to clang. Path
# components are ignored, as well as any command before the compiler name (for
# instance "/some/path/ccache /other/path/arm-linux-gnueabihf-").
# We try to extract any ccache command if present.
#$(info $$sm is [${sm}])
clang-target	:= $(patsubst %-,%,$(notdir $(lastword $(CROSS_COMPILE_$(sm)))))
#$(info $$clang-target is [${clang-target}])
ccache-cmd	:= $(if $(findstring ccache,$(CROSS_COMPILE_$(sm))),$(firstword $(CROSS_COMPILE_$(sm))) ,)
#$(info $$ccache-cmd is [${ccache-cmd}])

#CC$(sm)2		:= $(ccache-cmd)clang --target=aarch4-linux-android
#$(info $$CC${sm}2 is [${CC${sm}2}])
CC$(sm)		:= $(ccache-cmd)clang --target=$(clang-target)
# Due to the absence of clang-cpp in AOSP's prebuilt version of clang,
# use the equivalent command of 'clang -E'
#$(info $$CC${sm} is [${CC${sm}}])
CPP$(sm)	:= $(ccache-cmd)clang --target=$(clang-target) -E
#$(info $$CPP${sm} is [${CPP${sm}}])
LD$(sm)		:= $(ccache-cmd)ld.lld
#$(info $$LD${sm} is [${LD${sm}}])

AR$(sm)		:= $(ccache-cmd)llvm-ar
NM$(sm)		:= llvm-nm
OBJCOPY$(sm)	:= llvm-objcopy
OBJDUMP$(sm)	:= llvm-objdump
READELF$(sm)	:= llvm-readelf

nostdinc$(sm)	:= -nostdinc -isystem $(shell $(CC$(sm)) \
			-print-file-name=include 2> /dev/null)

comp-cflags-warns-clang := -Wno-language-extension-token \
			 -Wno-gnu-zero-variadic-macro-arguments

#$(info $$arch-bits-${sm} is [${arch-bits-${sm}}])
#$(info $$CFLAGS${arch-bits-${sm}} is [${CFLAGS${arch-bits-${sm}}}])
#$(info $$comp-cflags${sm} is [${comp-cflags${sm}}])
# Note, the currently used compiler runtime library may be libgcc.a or
# libclang_rt.builtins.*.a depending on the compiler build-time configuration.
libgcc$(sm)  	:= $(shell $(CC$(sm)) $(CFLAGS$(arch-bits-$(sm))) $(comp-cflags$(sm)) \
			-rtlib=compiler-rt -print-libgcc-file-name 2> /dev/null)
#$(info $$libgcc${sm}  is [${libgcc${sm}}])

# Core ASLR relies on the executable being ready to run from its preferred load
# address, because some symbols are used before the MMU is enabled and the
# relocations are applied.
ldflag-apply-dynamic-relocs := --apply-dynamic-relocs

# Define these to something to discover accidental use
CC		:= false
CPP		:= false
LD		:= false
AR		:= false
NM		:= false
OBJCOPY		:= false
OBJDUMP		:= false
READELF		:= false
nostdinc	:= --bad-nostdinc-variable
libgcc  	:= --bad-libgcc-variable

