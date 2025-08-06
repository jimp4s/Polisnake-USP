# Makefile Final e Definitivo (v15)
# Contém TODOS os arquivos-fonte e as flags de linkagem corretas.

RPI_VERSION = 2
GCC_PREFIX = arm-none-eabi-
TARGET = kernel.img

# Caminhos corretos para os arquivos de cabeçalho (.h)
INCLUDE_DIRS = -Iinclude -Ienv/include -Ienv/include/uspienv

# Lista completa de TODOS os arquivos-fonte (.c e .S) da sua biblioteca.
SOURCES = \
    kernel.c \
    lib/uspilibrary.c lib/usbkeyboard.c lib/usbdevice.c lib/usbfunction.c \
    lib/usbrequest.c lib/usbstandardhub.c lib/string.c lib/util.c lib/keymap.c \
    lib/synchronize.c lib/devicenameservice.c lib/dwhcidevice.c \
    lib/dwhciframeschednper.c lib/dwhciframeschednsplit.c lib/dwhciframeschedper.c \
    lib/dwhciregister.c lib/dwhcirootport.c lib/dwhcixferstagedata.c \
    lib/usbdevicefactory.c lib/usbendpoint.c lib/usbconfigparser.c \
    lib/usbmouse.c lib/usbmassdevice.c lib/lan7800.c lib/smsc951x.c \
    lib/usbgamepad.c lib/usbmidi.c lib/usbstring.c lib/macaddress.c \
    env/lib/bcmframebuffer.c env/lib/bcmmailbox.c env/lib/bcmpropertytags.c \
    env/lib/chargenerator.c env/lib/assert.c env/lib/interrupt.c \
    env/lib/logger.c env/lib/memory.c env/lib/screen.c env/lib/sysinit.c \
    env/lib/timer.c env/lib/uspibind.c env/lib/uspienv.c env/lib/alloc.c \
    env/lib/memio.c env/lib/pagetable.c env/lib/debug.c env/lib/exceptionhandler.c \
    env/lib/startup.S env/lib/exceptionstub.S env/lib/delayloop.S

# Opções de Compilação
CFLAGS = -g -O2 -ffreestanding -nostdlib $(INCLUDE_DIRS) -DRPI_VERSION=$(RPI_VERSION) -march=armv7-a -DARM_LOCAL_MAILBOX3_SET0=0

# Opções de Linkagem - A CORREÇÃO CRUCIAL ESTÁ AQUI
LDFLAGS = -T env/uspienv.ld -nostdlib -nostartfiles

# --- Regras de Compilação (NÃO ALTERAR) ---
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
	# Usamos gcc para linkar e adicionamos -lgcc para as funções auxiliares.
	$(GCC_PREFIX)gcc $(LDFLAGS) $(OBJS) -lgcc -o $(basename $@).elf
	$(GCC_PREFIX)objcopy $(basename $@).elf -O binary $@
clean:
	rm -rf build $(TARGET) *.elf