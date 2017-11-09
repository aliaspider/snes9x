
TARGET = snes9x_module.a
DEBUG = 0

platform ?= linux

BUILD_DIR = objs/$(platform)

ifeq ($(DEBUG),1)
   BUILD_DIR := $(BUILD_DIR)-dbg
endif

all: $(TARGET)

ifeq ($(platform),3ds)
   ifeq ($(strip $(DEVKITPRO)),)
      $(error "Please set DEVKITPRO in your environment. export DEVKITPRO=<path to>devkitpro")
   endif
   CFLAGS += -I$(DEVKITPRO)/libctru/include -I$(DEVKITPRO)/portlibs/armv6k/include
   LIBDIRS := -L. -L$(DEVKITPRO)/libctru/lib -L $(DEVKITPRO)/portlibs/armv6k/lib
   ARCH  := -march=armv6k -mtune=mpcore -mfloat-abi=hard -marm -mfpu=vfp -mtp=soft
   CFLAGS += -mword-relocations -ffast-math -Werror=implicit-function-declaration $(ARCH)
   CFLAGS += -DARM11 -D_3DS
   CXXFLAGS := $(CFLAGS) -fno-rtti -fno-exceptions -std=gnu++11
   CFLAGS   += -std=gnu99 -ffast-math
   ASFLAGS	:=  -g $(ARCH) -O3
   #PATH := $(PATH):$(DEVKITPRO)/devkitARM/bin

   CC      := arm-none-eabi-gcc
   CXX     := arm-none-eabi-g++
   AS      := arm-none-eabi-as
   AR      := arm-none-eabi-ar
   OBJCOPY := arm-none-eabi-objcopy
   STRIP   := arm-none-eabi-strip
   NM      := arm-none-eabi-nm
   LD      := $(CXX)
endif

OBJS :=
OBJS += interface.o
OBJS += apu/apu.o
OBJS += apu/bapu/dsp/sdsp.o
OBJS += apu/bapu/dsp/SPC_DSP.o
OBJS += apu/bapu/smp/smp.o
OBJS += apu/bapu/smp/smp_state.o
OBJS += bsx.o
OBJS += c4.o
OBJS += c4emu.o
OBJS += cheats.o
OBJS += cheats2.o
OBJS += clip.o
OBJS += conffile.o
OBJS += controls.o
OBJS += cpu.o
OBJS += cpuexec.o
OBJS += cpuops.o
OBJS += crosshairs.o
OBJS += dma.o
OBJS += dsp.o
OBJS += dsp1.o
OBJS += dsp2.o
OBJS += dsp3.o
OBJS += dsp4.o
OBJS += fxinst.o
OBJS += fxemu.o
OBJS += gfx.o
OBJS += globals.o
OBJS += logger.o
OBJS += memmap.o
OBJS += msu1.o
OBJS += movie.o
OBJS += obc1.o
OBJS += ppu.o
OBJS += stream.o
OBJS += sa1.o
OBJS += sa1cpu.o
OBJS += screenshot.o
OBJS += sdd1.o
OBJS += sdd1emu.o
OBJS += seta.o
OBJS += seta010.o
OBJS += seta011.o
OBJS += seta018.o
OBJS += snapshot.o
OBJS += snes9x.o
OBJS += spc7110.o
OBJS += srtc.o
OBJS += tile.o
OBJS += filter/2xsai.o
OBJS += filter/blit.o
OBJS += filter/epx.o
OBJS += filter/hq2x.o
OBJS += filter/snes_ntsc.o
OBJS += statemanager.o

OBJS := $(addprefix $(BUILD_DIR)/,$(OBJS))

ifeq ($(DEBUG),1)
   FLAGS += -g -O0
else
   FLAGS += -O3
endif

FLAGS += -Wall -Wextra -Werror
FLAGS += -Wno-sign-compare
FLAGS += -Wno-misleading-indentation
FLAGS += -Wno-unused-parameter
FLAGS += -Wno-unused-variable
FLAGS += -Wno-unused-value
FLAGS += -Wno-unused-function
FLAGS += -Wno-unused-but-set-variable
FLAGS += -Wno-int-to-pointer-cast
FLAGS += -Werror=implicit-function-declaration -Werror=incompatible-pointer-types
FLAGS += -Wno-array-bounds
FLAGS += -fomit-frame-pointer

#FLAGS += -DHAVE_STRINGS_H
FLAGS += -DHAVE_STDINT_H
#FLAGS += -DPTR_NOT_INT
FLAGS += -DRIGHTSHIFT_IS_SAR
FLAGS += -D__WIN32_LIBSNES__

FLAGS += -I. -Iapu/ -Iapu/bapu -Ijma/ -Ifilter/

CFLAGS += $(FLAGS)
CXXFLAGS += $(FLAGS) -fno-exceptions -fno-rtti


$(BUILD_DIR)/$(TARGET): $(OBJS) .lastbuild
	touch .lastbuild
	$(AR) rcs $@ $(OBJS)

$(TARGET): $(BUILD_DIR)/$(TARGET)
	cp $< $@

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $< $(CFLAGS) -MT $@ -MMD -MP -MF $(BUILD_DIR)/$*.depend -c -o $@

$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CC) $< $(CXXFLAGS) -MT $@ -MMD -MP -MF $(BUILD_DIR)/$*.depend -c -o $@

.lastbuild: ;

clean:
#	rm -rf objs
	rm -f $(OBJS) $(OBJS:.o=.depend)
	rm -f $(BUILD_DIR)/$(TARGET) $(TARGET) .lastbuild


-include $(OBJS:.o=.depend)
