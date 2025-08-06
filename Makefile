# Makefile Final e Definitivo (v9)
# Construído sob medida para a sua estrutura de arquivos.

RPI_VERSION = 2
GCC_PREFIX = arm-none-eabi-
TARGET = kernel.img

# A correção crucial: Apontamos para "env/include", o que permite que
# o compilador encontre o subdiretório "uspienv".
INCLUDE_DIRS = -Iinclude -Ienv/include

# Lista completa de arquivos-fonte (.c e .S) da sua biblioteca.
SOURCES = lib/uspilibrary.c \
          lib/usbkeyboard.c \
          lib/usbdevice.c \
          lib/usbfunction.c \
          lib/usbrequest.c \
          lib/usbstandardhub.c \
          lib/string.c \
          lib/util.c \
          lib/keymap.c \
          lib/synchronize.c \
          lib/devicenameservice.c \
          lib/dwhcidevice.c \
          lib/dwhciframeschednper.c \
          lib/dwhciframeschednsplit.c \
          lib/dwhciframeschedper.c \
          lib/dwhciregister.c \
          lib/dwhcirootport.c \
          lib/dwhcixferstagedata.c \
          env/lib/bcmframebuffer.c \
          env/lib/bcmmailbox.c \
          env/lib/bcmpropertytags.c \
          env/lib/chargenerator.c \
          env/lib/assert.c \
          env/lib/interrupt.c \
          env/lib/logger.c \
          env/lib/memory.c \
          env/lib/screen.c \
          env/lib/sysinit.c \
          env/lib/timer.c \
          env/lib/uspibind.c \
          env/lib/uspienv.c \
          env/$(RPI_VERSION)/rpi-stub.S \
          env/lib/delayloop.S \
          kernel.c

# OPÇÕES DE COMPILAÇÃO E LINKER
CFLAGS = -g -O2 -ffreestanding -nostdlib $(INCLUDE_DIRS) -DRPI_VERSION=$(RPI_VERSION)
LDFLAGS = -T env/uspienv.ld -nostdlib

# --- REGRAS DE COMPILAÇÃO (NÃO ALTERAR) ---
OBJS = $(patsubst %.c,build/%.o,$(filter %.c,$(SOURCES))) \
       $(patsubst %.S,build/%.o,$(filter %.S,$(SOURCES)))

.PHONY: all clean
all: $(TARGET)

build/%.o: %.c
	@mkdir -p $(@D)
	$(GCC_PREFIX)gcc $(CFLAGS) -c $< -o $@

build/%.o: %.S
	@mkdir -p $(@D)
	$(GCC_PREFIX)gcc $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJS)
	$(GCC_PREFIX)ld $(LDFLAGS) $(OBJS) -o $(basename $@).elf
	$(GCC_PREFIX)objcopy $(basename $@).elf -O binary $@

clean:
	rm -rf build $(TARGET) *.elf