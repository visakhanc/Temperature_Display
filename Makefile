# Hey Emacs, this is a -*- makefile -*-

.PHONY:	all build elf hex eep lss sym program coff extcoff clean depend

MCU = atmega8
F_CPU = 1000000
FORMAT = ihex
TARGET = main
SRC = $(TARGET).c
OPT = s

# List any extra directories to look for include files here.
#     Each directory must be seperated by a space.
#     Use forward slashes for directory separators.
#     For a directory that has spaces, enclose it in quotes.
CINCS = -I.

# List any extra directories to look for libraries here.
#     Each directory must be seperated by a space.
#     Use forward slashes for directory separators.
#     For a directory that has spaces, enclose it in quotes.


CSTANDARD = -std=gnu99
CDEFS = -DF_CPU=$(F_CPU)UL
CDEBUG = -g
CWARN = -Wall -Wstrict-prototypes
CTUNING = -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
#CEXTRA = -Wa,-adhlns=$(<:.c=.lst)
ALL_CFLAGS = -mmcu=$(MCU) $(CDEBUG) $(CINCS) $(CDEFS) -O$(OPT) $(CWARN) $(CSTANDARD) $(CTUNING)


LDFLAGS =  -Wl,-Map=$(TARGET).map,--cref
LDFLAGS += $(patsubst %,-L%,$(EXTRALIBDIRS))
#LDFLAGS += -lRFM70
LDFLAGS += -Wl,--section-start=.text=0x0000


####### AVRDUDE #######

AVRDUDE_PROGRAMMER = usbasp
AVRDUDE_PORT = usb
AVRDUDE_WRITE_FLASH = -U flash:w:$(TARGET).hex
#AVRDUDE_WRITE_EEPROM = -U eeprom:w:$(TARGET).eep

# High and Low fuse settings for AVR
FUSE_H = 0xd9	
FUSE_L = 0xe1

AVRDUDE_FLAGS = -B12 -p $(MCU) -P $(AVRDUDE_PORT) -c $(AVRDUDE_PROGRAMMER)

###### COMPILE AND LINK ########
CC = avr-gcc
OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump
SIZE = avr-size
NM = avr-nm
AVRDUDE = avrdude
REMOVE = rm -f
MV = mv -f
SIZE = avr-size
AR = avr-ar rcs

# Define all object files.
OBJ = $(SRC:.c=.o)  


# Define all listing files.
LST = $(SRC:.c=.lst)


# Default target. (lib for compiling to static library)

all: elf hex eep lss size
#all: lib

elf: $(TARGET).elf
hex: $(TARGET).hex
eep: $(TARGET).eep
lss: $(TARGET).lss 
sym: $(TARGET).sym
LIBNAME=lib$(TARGET).a
lib: $(LIBNAME)

size: 
	$(SIZE) --mcu=$(MCU) --format=avr $(TARGET).elf

# Program the device.  $(TARGET).hex $(TARGET).eep
program: 
	$(AVRDUDE) $(AVRDUDE_FLAGS) $(AVRDUDE_WRITE_FLASH) $(AVRDUDE_WRITE_EEPROM)

fuse:
	$(AVRDUDE) -U hfuse:w:$(FUSE_H):m -U lfuse:w:$(FUSE_L):m
	
.SUFFIXES: .elf .hex .eep .lss .sym

.elf.hex:
	$(OBJCOPY) -O $(FORMAT) -R .eeprom $< $@

.elf.eep:
	-$(OBJCOPY) -j .eeprom --set-section-flags=.eeprom="alloc,load" \
	--change-section-lma .eeprom=0 -O $(FORMAT) $< $@

# Create extended listing file from ELF output file.
.elf.lss:
	$(OBJDUMP) -h -S $< > $@

# Create a symbol table from ELF output file.
.elf.sym:
	$(NM) -n $< > $@


# Link:
# create ELF output file from object files.
$(TARGET).elf: $(OBJ)
	$(CC) $(ALL_CFLAGS) $(OBJ) --output $@ $(LDFLAGS)

%.a: $(OBJ)
	$(AR) $@ $(OBJ)
	
# Compile: 
# create object files from C source files.
.c.o:
	$(CC) -c $(ALL_CFLAGS) $< -o $@ 

# Target: clean project.
clean:
	$(REMOVE) $(TARGET).hex $(TARGET).eep $(TARGET).cof $(TARGET).elf \
	$(TARGET).map $(TARGET).sym $(TARGET).lss \
	$(OBJ) $(LST) $(SRC:.c=.s) $(SRC:.c=.d)
