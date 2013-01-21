

# Hey Emacs, this is a -*- makefile -*-

#
# WinARM template makefile 
# by Martin Thomas, Kaiserslautern, Germany 
# <eversmith@heizung-thomas.de>
# Modified for the STM32F103 Medium Density devices
# by Peter Harrison, May 2009
# Modified for library version 3.1 and High Density Devices
# by Francois Gervais, August 2009
# Modified for the STM32F4 devices
# by Flexz, 2011
#
# based on the WinAVR makefile written by Eric B. Weddington, Jörg Wunsch, et al.
# Released to the Public Domain
# Please read the make user manual!
#
#
# On command line:
#
# make all = Make software.
#
# make clean = Clean out built project files.
#
# (TODO: make program = Download the hex file to the device)
#
# (TODO: make filename.s = Just compile filename.c into the assembler code only)
#
# To rebuild project do "make clean" then "make all".
#

# Toolchain prefix (i.e arm-none-eabi -> arm-none-eabi-gcc.exe)
TCHAIN = arm-none-eabi

# MCU name and submodel
MCU      = cortex-m4
#SUBMDL   = STM32F2XX
SUBMDL   = STM32F4XX
DENSITY  = STM32F20X

# must be yes - only THUMB2 on M3 no ARM:
USE_THUMB_MODE = YES
PALTFORM = ./platform/stm32f4
#PALTFORM = ./platform/stm32f4_150MHz

## Exception-Vector placement not used so far in M3-examples
## (placement settings ignored when using "RAM_RUN")
## - Exception vectors in ROM:
#VECTOR_LOCATION=VECTORS_IN_ROM
## - Exception vectors in RAM:
##VECTOR_LOCATION=VECTORS_IN_RAM


# The following are all required if you are to use the ST peripheral driver library V3
# The next two should be copied into the project directory
# While these two can stay in the CM3 folder as part of the library
#SRC += core_cm4.c
SRC += system.c
SRC += syscalls.c stm32f4xx_dma.c stm32f4xx_tim.c stm32f4xx_gpio.c stm32f4xx_i2c.c stm32f4xx_fsmc.c stm32f4xx_usart.c
SRC += stm32f4xx_adc.c misc.c stm32f4xx_sdio.c stm32f4xx_exti.c stm32f4xx_syscfg.c stm32f4xx_rcc.c  stm32f4xx_dcmi.c
SRC += delay.c
#SRC += LCD.c AsciiLib.c
#STM32 USB FS Lib
SRC += usbd_core.c usbd_ioreq.c usbd_req.c usbd_usr.c usb_dcd.c usb_core.c usb_dcd_int.c
SRC += usbd_desc.c usbd_cdc_vcp.c usbd_cdc_core.c

# List C source files here which must be compiled in ARM-Mode.
# use file-extension c for "c-only"-files
SRCARM  = 

# List C++ source files here.
# use file-extension cpp for C++-files (use extension .cpp)
#CPPSRC = $(wildcard *.cpp) 
CPPSRC = main.cpp nvic.cpp
CPPSRC += usb.cpp
CPPSRC += la_sampling.cpp sump.cpp

# List C++ source files here which must be compiled in ARM-Mode.
# use file-extension cpp for C++-files (use extension .cpp)
CPPSRCARM = 

# List Assembler source files here.
# Make them always end in a capital .S.  Files ending in a lowercase .s
# will not be considered source files but generated files (assembler
# output from the compiler), and will be deleted upon "make clean"!
# Even though the DOS/Win* filesystem matches both .s and .S the same,
# it will preserve the spelling of the filenames, and gcc itself does
# care about how the name is spelled on its command-line.
ASRC = $(PALTFORM)/startup.S

# List Assembler source files here which must be assembled in ARM-Mode..
ASRCARM  = 

## Output format. (can be ihex or binary)
## (binary i.e. for openocd, lmiflash and SAM-BA, hex i.e. for lpc21isp and uVision)
#FORMAT = ihex
FORMAT = binary

## Configre flash tool - supported: UVISION, OPENOCD, LMIFLASH
## use FORMAT = binary for OpenOCD and lmiflash
## use FORMAT = ihex for uVision
#FLASH_TOOL = UVISION
#FLASH_TOOL = OPENOCD
#FLASH_TOOL = LMIFLASH
FLASH_TOOL = STM32LOADER

# Optimization level, can be [0, 1, 2, 3, s]. 
# 0 = turn off optimization. s = optimize for size.
# (Note: 3 is not always the best optimization level. See avr-libc FAQ.)
OPT = 3

# Debugging format.
# Native formats for AVR-GCC's -g are stabs [default], or dwarf-2.
# AVR (extended) COFF requires stabs, plus an avr-objcopy run.
#DEBUG = stabs
DEBUG = dwarf-2

# Path to Linker-Scripts
LINKERSCRIPTPATH = ./

# list some places to look for source files (PH May 2009)
# the ST peripheral driver library sources must be findable
#STLIBS   = G:/Work/ST/libs
LIBSTM32 = $(STLIBS)/STM32F4xx_DSP_StdPeriph_Lib_V1.0.1/Libraries
#CM3      = $(LIBSTM32)/CMSIS/CM3/CoreSupport
CM4      = $(LIBSTM32)/CMSIS
CM3_DEV  = $(LIBSTM32)/CMSIS/Device/ST/STM32F4xx
DRIVERS  = $(LIBSTM32)/STM32F4xx_StdPeriph_Driver
STARTUP  = $(LIBSTM32)/CMSIS/Core/CM3/startup/gcc
#USBCL 	 = ./usb/raw
USBCL 	 = ./usb/vcp
LA       = ./la
USBOTG   = $(STLIBS)/STM32_USB-Host-Device_Lib_V2.1.0/Libraries/STM32_USB_OTG_Driver
USBLIB   = $(STLIBS)/STM32_USB-Host-Device_Lib_V2.1.0/Libraries/STM32_USB_Device_Library/Core
ONBOARD  = ./onboard
#BOOST	 = D:/Stuff/boost

#vpath %.S $(LIBSPEEX)/STM32/libspeex/gcc
vpath %.c $(DRIVERS)/src $(USBCL) $(USBCDC)/src $(USBOTG)/src $(CM3) $(CM3_DEV) $(STARTUP) ./print $(USBLIB)/src $(PALTFORM) ./lcd ./camera
vpath %.cpp ./print $(USBLIB)/src ./user $(USBCL) ./board $(MODS) $(LA)

# List any extra directories to look for include files here.
# Each directory must be separated by a space.
# Make sure we can find the peripheral driver library include files (PH May 2009)
EXTRAINCDIRS = $(DRIVERS)/inc $(USBCL) $(USBLIB)/inc $(USBCDC)/inc $(USBOTG)/inc $(CM3) $(CM3_DEV)/Include $(SVDLIB)  ./print $(PALTFORM) $(CM4)/Include ./lcd $(LA)

# List any extra directories to look for library files here.
# Each directory must be separated by a space.
EXTRA_LIBDIRS = 

## Using the Atmel AT91_lib produces warning with
## the default warning-levels. 
## yes - disable these warnings; no - keep default settings
#AT91LIBNOWARN = yes
AT91LIBNOWARN = no

# Compiler flag to set the C Standard level.
# c89   - "ANSI" C
# gnu89 - c89 plus GCC extensions
# c99   - ISO C99 standard (not yet fully implemented)
# gnu99 - c99 plus GCC extensions
CSTANDARD = -std=gnu99

# Place -D or -U options for C here
CDEFS +=  -Dgcc -D$(DENSITY) -D$(SUBMDL) -DHSE_VALUE=8000000 
#CDEFS +=  -Dgcc -DSTM32F10X_MD
CDEFS +=  -DCFG_LINKER_HAS_ISR_SECTION -DUSE_STDPERIPH_DRIVER 
CDEFS +=  -DHAVE_CONFIG_H
CDEFS += -DFALSE=false -DTRUE=true

# Place -I options here
CINCS =

# Place -D or -U options for ASM here
ADEFS =  

ifdef VECTOR_LOCATION
CDEFS += -D$(VECTOR_LOCATION)
ADEFS += -D$(VECTOR_LOCATION)
endif


# Compiler flags.

ifeq ($(USE_THUMB_MODE),YES)
THUMB    = -mthumb
THUMB_IW = -mthumb-interwork
else 
THUMB    = 
THUMB_IW = 
endif

#  -g*:          generate debugging information
#  -O*:          optimization level
#  -f...:        tuning, see GCC manual and avr-libc documentation
#  -Wall...:     warning level
#  -Wa,...:      tell GCC to pass this to the assembler.
#    -adhlns...: create assembler listing
#
# Flags for C and C++ (arm-elf-gcc/arm-elf-g++)
CFLAGS = -g$(DEBUG) -mfpu=fpv4-sp-d16 -mfloat-abi=softfp
CFLAGS += $(CDEFS) $(CINCS)
CFLAGS += -O$(OPT)
#CFLAGS += -ffunction-sections -fdata-sections
CFLAGS += -Wall -Wimplicit 
CFLAGS += -Wcast-align
CFLAGS += -Wpointer-arith -Wswitch
#CFLAGS += -Wredundant-decls -Wreturn-type -Wshadow -Wunused
CFLAGS += -Wredundant-decls -Wreturn-type -Wunused
CFLAGS += -Wa,-adhlns=$(subst $(suffix $<),.lst,$<) 
CFLAGS += $(patsubst %,-I%,$(EXTRAINCDIRS))

# flags only for C
CONLYFLAGS += -Wnested-externs 
CONLYFLAGS += $(CSTANDARD)

ifneq ($(AT91LIBNOWARN),yes)
#AT91-lib warnings with:
CFLAGS += -Wcast-qual
#CONLYFLAGS += -Wmissing-prototypes 
#CONLYFLAGS += -Wstrict-prototypes
#CONLYFLAGS += -Wmissing-declarations
endif

# flags only for C++ (arm-elf-g++)
#CPPFLAGS = -fno-rtti -fno-exceptions -fno-strict-aliasing
CPPFLAGS = -fno-exceptions -fno-strict-aliasing

# Assembler flags.
#  -Wa,...:    tell GCC to pass this to the assembler.
#  -ahlns:     create listing
#  -g$(DEBUG): have the assembler create line number information
ASFLAGS = $(ADEFS) -Wa,-adhlns=$(<:.S=.lst),--g$(DEBUG)


#Additional libraries.

# Extra libraries
#    Each library-name must be seperated by a space.
#    To add libxyz.a, libabc.a and libefsl.a: 
#    EXTRA_LIBS = xyz abc efsl
#EXTRA_LIBS = efsl
#EXTRA_LIBS = luminary

#Support for newlibc-lpc (file: libnewlibc-lpc.a)
#NEWLIBLPC = -lnewlib-lpc

MATH_LIB = -lm

CPLUSPLUS_LIB = -lstdc++

# Linker flags.
#  -Wl,...:     tell GCC to pass this to linker.
#    -Map:      create map file
#    --cref:    add cross reference to  map file
#LDFLAGS = -nostartfiles -Wl,-Map=$(TARGET).map,--cref,--gc-sections
LDFLAGS = -nostartfiles -Wl,-Map=$(TARGET).map,--cref,--gc-sections 
LDFLAGS += -lc# -mfpu=fpv4-sp-d16 --mfloat-abi=hard
LDFLAGS += $(NEWLIBLPC) $(MATH_LIB)
LDFLAGS += -lc -lgcc
LDFLAGS += $(CPLUSPLUS_LIB)
LDFLAGS += $(patsubst %,-L%,$(EXTRA_LIBDIRS))
LDFLAGS += $(patsubst %,-l%,$(EXTRA_LIBS))

# Set Linker-Script Depending On Selected Memory and Controller
# Just use the STM32F103RBT6 linker script (PH May 2009)
# This needs making processor dependant
LDFLAGS +=-T$(LINKERSCRIPTPATH)stm32f4xx_flash.ld
#LDFLAGS +=-T$(LINKERSCRIPTPATH)generic.ld

# ---------------------------------------------------------------------------

# Define programs and commands.
SHELL = sh
CC = $(TCHAIN)-gcc
CPP = $(TCHAIN)-g++
AR = $(TCHAIN)-ar
OBJCOPY = $(TCHAIN)-objcopy #--remove-section=.ccm_data --remove-section=.pram_data
OBJDUMP = $(TCHAIN)-objdump #--remove-section=.ccm_data --remove-section=.pram_data
SIZE = $(TCHAIN)-size
NM = $(TCHAIN)-nm
REMOVE = rm -f
REMOVEDIR = rm -r
COPY = cp

# Define Messages
# English
MSG_ERRORS_NONE = Errors: none
MSG_BEGIN = "-------- begin --------"
MSG_END = --------  end  --------
MSG_SIZE_BEFORE = Size before: 
MSG_SIZE_AFTER = Size after:
MSG_FLASH = Creating load file for Flash:
MSG_EXTENDED_LISTING = Creating Extended Listing:
MSG_SYMBOL_TABLE = Creating Symbol Table:
MSG_LINKING = [Linking]
MSG_COMPILING = [Compiling C]
MSG_COMPILING_ARM = "Compiling C (ARM-only):"
MSG_COMPILINGCPP = [Compiling C++]
MSG_COMPILINGCPP_ARM = "Compiling C++ (ARM-only):"
MSG_ASSEMBLING = Assembling:
MSG_ASSEMBLING_ARM = "Assembling (ARM-only):"
MSG_CLEANING = Cleaning project:
MSG_FORMATERROR = Can not handle output-format
MSG_LPC21_RESETREMINDER = You may have to bring the target in bootloader-mode now.

# Define all object files.
COBJ      = $(SRC:.c=.o) 
AOBJ      = $(ASRC:.S=.o)
COBJARM   = $(SRCARM:.c=.o)
AOBJARM   = $(ASRCARM:.S=.o)
CPPOBJ    = $(CPPSRC:.cpp=.o) 
CPPOBJARM = $(CPPSRCARM:.cpp=.o)


# Define all listing files.
LST = $(ASRC:.S=.lst) $(ASRCARM:.S=.lst) $(SRC:.c=.lst) $(SRCARM:.c=.lst)
LST += $(CPPSRC:.cpp=.lst) $(CPPSRCARM:.cpp=.lst)

# Compiler flags to generate dependency files.
### GENDEPFLAGS = -Wp,-M,-MP,-MT,$(*F).o,-MF,.dep/$(@F).d
GENDEPFLAGS = -MD -MP -MF .dep/$(@F).d

# Combine all necessary flags and optional flags.
# Add target processor to flags.
ALL_CFLAGS  = -mcpu=$(MCU) $(THUMB_IW) -I. $(CFLAGS) $(GENDEPFLAGS)
ALL_ASFLAGS = -mcpu=$(MCU) $(THUMB_IW) -I. -x assembler-with-cpp $(ASFLAGS)


# Default target.
all: begin gccversion sizebefore build sizeafter finished end

ifeq ($(FORMAT),ihex)
build: elf hex sym
hex: $(TARGET).hex
IMGEXT=hex
else 
ifeq ($(FORMAT),binary)
build: elf bin sym
bin: $(TARGET).bin
IMGEXT=bin
else 
$(error "$(MSG_FORMATERROR) $(FORMAT)")
endif
endif

elf: $(TARGET).elf
lss: $(TARGET).lss 
sym: $(TARGET).sym

# Eye candy.
begin:
	@echo --
	@echo $(MSG_BEGIN)

finished:
	@echo $(MSG_ERRORS_NONE)

end:
	@echo $(MSG_END)
	@echo


# Display size of file.
HEXSIZE = $(SIZE) --target=$(FORMAT) $(TARGET).hex
ELFSIZE = $(SIZE) -A $(TARGET).elf
sizebefore:
#	@if [ -f $(TARGET).elf ]; then echo; echo $(MSG_SIZE_BEFORE); $(ELFSIZE); echo; fi

sizeafter:
	@if [ -f $(TARGET).elf ]; then echo; echo $(MSG_SIZE_AFTER); $(ELFSIZE); echo; fi
#	echo $(MSG_SIZE_AFTER)
#	$(ELFSIZE)

# Display compiler version information.
gccversion : 
	@$(CC) --version


# Program the device with Keil's uVision (needs configured uVision-Workspace). 
program: $(TARGET).$(IMGEXT)
	@echo
ifeq ($(FLASH_TOOL),UVISION)
	@echo "Flash-programming with uVision"
	C:\Keil\uv3\Uv3.exe -f ../../tools/uvisionflash811.uv2 -ouvisionflash.txt
else
ifeq ($(FLASH_TOOL),OPENOCD)
	@echo "Flash-programming with OpenOCD"
	cp $(TARGET).bin ../../tools/tmpflash.bin
	cd ../../tools && make program
	rm ../../tools/tmpflash.bin
else
ifeq ($(FLASH_TOOL),LMIFLASH)
	@echo "Flash-programming with lmiflash"
	../../tools/lmiflash.exe -f $(TARGET).$(IMGEXT) -v -r
else
ifeq ($(FLASH_TOOL),STM32LOADER)
	@echo "Flash programming with stm32loader.py"
	stm32loader.py -evw $(TARGET).$(IMGEXT)
endif
endif
endif
endif

# Create final output file (.hex) from ELF output file.
%.hex: %.elf
	@echo
	@echo $(MSG_FLASH) $@
	$(OBJCOPY) -O $(FORMAT) $< $@
	
# Create final output file (.bin) from ELF output file.
%.bin: %.elf
	@echo
	@echo $(MSG_FLASH) $@
	$(OBJCOPY) -O $(FORMAT) $< $@


# Create extended listing file from ELF output file.
# testing: option -C
%.lss: %.elf
	@echo
	@echo $(MSG_EXTENDED_LISTING) $@
	$(OBJDUMP) -h -S -D $< > $@


# Create a symbol table from ELF output file.
%.sym: %.elf
	@echo
	@echo $(MSG_SYMBOL_TABLE) $@
	$(NM) -n $< > $@


# Link: create ELF output file from object files.
.SECONDARY : $(TARGET).elf
.PRECIOUS : $(AOBJARM) $(AOBJ) $(COBJARM) $(COBJ) $(CPPOBJ) $(CPPOBJARM)
%.elf:  $(AOBJARM) $(AOBJ) $(COBJARM) $(COBJ) $(CPPOBJ) $(CPPOBJARM)
	@echo
	@echo $(MSG_LINKING) $@
	@$(CC) $(THUMB) $(ALL_CFLAGS) $(AOBJARM) $(AOBJ) $(COBJARM) $(COBJ) $(CPPOBJ) $(CPPOBJARM) --output $@ $(LDFLAGS)
#	arm-elf-gcc $(THUMB) $(ALL_CFLAGS) $(AOBJARM) $(AOBJ) $(COBJARM) $(COBJ) $(CPPOBJ) $(CPPOBJARM) --output $@ $(LDFLAGS)
#	@$(CPP) $(THUMB) $(ALL_CFLAGS) $(AOBJARM) $(AOBJ) $(COBJARM) $(COBJ) $(CPPOBJ) $(CPPOBJARM) --output $@ $(LDFLAGS)
## mist arm-elf-ld -Map=$(TARGET).map --cref --gc-sections $(AOBJARM) $(AOBJ) $(COBJARM) $(COBJ) $(CPPOBJ) $(CPPOBJARM) --output $@

# Compile: create object files from C source files. ARM/Thumb
$(COBJ) : %.o : %.c
#	@echo
	@echo $(MSG_COMPILING) $<
	@$(CC) -c $(THUMB) $(ALL_CFLAGS) $(CONLYFLAGS) $< -o $@ 
#	$(CPP) -c $(THUMB) $(ALL_CFLAGS) $(CPPFLAGS) $< -o $@ 

# Compile: create object files from C source files. ARM-only
$(COBJARM) : %.o : %.c
#	@echo
	@echo $(MSG_COMPILING_ARM) $<
	@$(CC) -c $(ALL_CFLAGS) $(CONLYFLAGS) $< -o $@ 
#	@$(CPP) -c $(ALL_CFLAGS) $(CPPFLAGS) $< -o $@ 

# Compile: create object files from C++ source files. ARM/Thumb
$(CPPOBJ) : %.o : %.cpp
#	@echo
	@echo $(MSG_COMPILINGCPP) $<
	$(CPP) -c $(THUMB) $(ALL_CFLAGS) $(CPPFLAGS) $< -o $@ 

# Compile: create object files from C++ source files. ARM-only
$(CPPOBJARM) : %.o : %.cpp
	@echo
	@echo $(MSG_COMPILINGCPP_ARM) $<
	$(CPP) -c $(ALL_CFLAGS) $(CPPFLAGS) $< -o $@ 


# Compile: create assembler files from C source files. ARM/Thumb
## does not work - TODO - hints welcome
##$(COBJ) : %.s : %.c
##	$(CC) $(THUMB) -S $(ALL_CFLAGS) $< -o $@


# Assemble: create object files from assembler source files. ARM/Thumb
$(AOBJ) : %.o : %.S
	@echo
	@echo $(MSG_ASSEMBLING) $<
	$(CC) -c $(THUMB) $(ALL_ASFLAGS) $< -o $@
#	$(CPP) -c $(THUMB) $(ALL_ASFLAGS) $< -o $@


# Assemble: create object files from assembler source files. ARM-only
$(AOBJARM) : %.o : %.S
	@echo
	@echo $(MSG_ASSEMBLING_ARM) $<
	$(CC) -c $(ALL_ASFLAGS) $< -o $@
#	$(CPP) -c $(ALL_ASFLAGS) $< -o $@


# Target: clean project.
clean: begin clean_list finished end


clean_list :
	@echo
	@echo $(MSG_CLEANING)
	$(REMOVE) $(TARGET).hex
	$(REMOVE) $(TARGET).bin
	$(REMOVE) $(TARGET).obj
	$(REMOVE) $(TARGET).elf
	$(REMOVE) $(TARGET).map
	$(REMOVE) $(TARGET).obj
	$(REMOVE) $(TARGET).a90
	$(REMOVE) $(TARGET).sym
	$(REMOVE) $(TARGET).lnk
	$(REMOVE) $(TARGET).lss
	$(REMOVE) $(COBJ)
	$(REMOVE) $(CPPOBJ)
	$(REMOVE) $(AOBJ)
	$(REMOVE) $(COBJARM)
	$(REMOVE) $(CPPOBJARM)
	$(REMOVE) $(AOBJARM)
	$(REMOVE) $(LST)
	$(REMOVE) $(SRC:.c=.s)
	$(REMOVE) $(SRC:.c=.d)
	$(REMOVE) $(SRCARM:.c=.s)
	$(REMOVE) $(SRCARM:.c=.d)
	$(REMOVE) $(CPPSRC:.cpp=.s) 
	$(REMOVE) $(CPPSRC:.cpp=.d)
	$(REMOVE) $(CPPSRCARM:.cpp=.s) 
	$(REMOVE) $(CPPSRCARM:.cpp=.d)
	$(REMOVE) .dep/*
#	$(REMOVEDIR) .dep
# remove files created by uVision during flash programming
	$(REMOVE) $(TARGET).hex.plg
	$(REMOVE) uvisionflash.txt


# Include the dependency files.
-include $(shell mkdir .dep 2>/dev/null) $(wildcard .dep/*)


# Listing of phony targets.
.PHONY : all begin finish end sizebefore sizeafter gccversion \
build elf hex bin lss sym clean clean_list program

