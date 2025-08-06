# Makefile para compilar o kernel.c com a biblioteca USPi (versão 2)
# Usando o driver de baixo nível usbkeyboard.h

# Altere para a sua versão da Pi. Opções: 1, 2, 3, 4
RPI_VERSION = 3

# Prefixo do compilador
GCC_PREFIX = arm-none-eabi-

# Nome do arquivo final (kernel7.img para Pi 2/3)
TARGET = kernel7.img
ifeq ($(RPI_VERSION), 4)
    TARGET = kernel7l.img
endif
ifeq ($(RPI_VERSION), 1)
	TARGET = kernel.img
endif

# Diretórios para procurar arquivos de cabeçalho (.h)
INCLUDE_DIRS = -Iinclude -Iuspi -Ienv/$(RPI_VERSION)

# Arquivos fonte C
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

# Regras de compilação (não precisa alterar abaixo desta linha)
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