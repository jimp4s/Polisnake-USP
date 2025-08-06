# Makefile Definitivo para o projeto Polisnake

# Altere para a sua versão da Pi. Opções: 1, 2, 3, 4
RPI_VERSION = 3

# Prefixo do compilador
GCC_PREFIX = arm-none-eabi-

# Nome do arquivo final
TARGET = kernel7.img
ifeq ($(RPI_VERSION), 4)
    TARGET = kernel7l.img
endif
ifeq ($(RPI_VERSION), 1)
	TARGET = kernel.img
endif

# Diretórios para procurar arquivos de cabeçalho (.h)
# Esta linha garante que o compilador encontre rpi-mailbox.h e todos os outros.
INCLUDE_DIRS = -Iinclude -Iuspi -Ienv/$(RPI_VERSION)

# Lista de todos os arquivos-fonte (.c) necessários para compilar.
SOURCES_C = kernel.c \
            uspi/uspi.c \
            uspi/uspistd.c \
            uspi/uspihub.c \
            uspi/usbkeyboard.c \
            uspi/usbmsc.c \
            uspi/usbdevice.c \
            uspi/usbdriver.c \
            uspi/usbhid.c \
            uspi/usbsys.c \
            env/$(RPI_VERSION)/rpi-stub.c \
            env/$(RPI_VERSION)/rpi-systimer.c \
            env/$(RPI_VERSION)/rpi-gpio.c \
            env/$(RPI_VERSION)/rpi-interrupts.c \
            env/$(RPI_VERSION)/rpi-mailbox.c \
            env/font.c

# Opções de compilação
CFLAGS = -g -O2 -ffreestanding -nostdlib $(INCLUDE_DIRS) -DRPI_VERSION=$(RPI_VERSION)
LDFLAGS = -T env/$(RPI_VERSION)/rpi.ld -nostdlib

# --- Regras de Compilação (não alterar abaixo) ---
OBJS = $(patsubst %.c,build/%.o,$(filter %.c,$(SOURCES_C)))

.PHONY: all clean

all: $(TARGET)

build/%.o: %.c
	@mkdir -p $(@D)
	$(GCC_PREFIX)gcc $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJS)
	$(GCC_PREFIX)ld $(LDFLAGS) $(OBJS) -o $(basename $@).elf
	$(GCC_PREFIX)objcopy $(basename $@).elf -O binary $@

clean:
	rm -rf build $(TARGET) *.elf