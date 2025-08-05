# Makefile para compilar o kernel.c com a biblioteca USPi
# Adaptado dos exemplos de rsta2

# Altere para a sua versão da Pi. Opções: 1, 2, 3, 4
RPI_VERSION = 3

# Prefixo do compilador
GCC_PREFIX = arm-none-eabi-

# Arquivos fonte
# Nosso kernel + fontes da uspi + fontes do ambiente (hardware)
SOURCES = kernel.c \
          uspi/uspi.c uspi/uspistd.c uspi/uspihub.c uspi/uspikeyboard.c \
          env/$(RPI_VERSION)/rpi-stub.c \
          env/$(RPI_VERSION)/rpi-systimer.c \
          env/$(RPI_VERSION)/rpi-gpio.c \
          env/$(RPI_VERSION)/rpi-interrupts.c \
          env/$(RPI_VERSION)/rpi-mailbox.c \
          env/font.c

# Nome do arquivo final
TARGET = kernel7.img
ifeq ($(RPI_VERSION), 4)
    TARGET = kernel7l.img # Ou kernel8.img para 64-bit
endif

# Opções de compilação
CFLAGS = -g -O2 -ffreestanding -nostdlib -Iinclude -Ienv/$(RPI_VERSION) -DRPI_VERSION=$(RPI_VERSION)
ASFLAGS = -g
LDFLAGS = -T env/$(RPI_VERSION)/rpi.ld -nostdlib

# Regras de compilação
OBJS = $(patsubst %.c,build/%.o,$(filter %.c,$(SOURCES))) \
       $(patsubst %.S,build/%.o,$(filter %.S,$(SOURCES)))

all: $(TARGET)

build/%.o: %.c
	@mkdir -p $(@D)
	$(GCC_PREFIX)gcc $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJS)
	$(GCC_PREFIX)ld $(LDFLAGS) $(OBJS) -o $(basename $@).elf
	$(GCC_PREFIX)objcopy $(basename $@).elf -O binary $@

clean:
	rm -rf build $(TARGET) *.elf