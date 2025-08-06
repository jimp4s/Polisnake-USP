/**
 * kernel.c (v14 - Final e Definitivo)
 * Inclui screen.h e usa main() como ponto de entrada.
 */

#include "uspienv.h"
#include "uspi.h"
#include "screen.h"  // Header que define ScreenInitialise, etc.

#define KBD_BUFFER_SIZE 256
static char kbd_buffer[KBD_BUFFER_SIZE];
static volatile unsigned int kbd_write_pos = 0, kbd_read_pos = 0;

void KeyPressedHandler(const char *pString)
{
	if (*pString != '\0')
	{
		kbd_buffer[kbd_write_pos] = *pString;
		kbd_write_pos = (kbd_write_pos + 1) % KBD_BUFFER_SIZE;
	}
}

char GetChar(void)
{
	if (kbd_read_pos == kbd_write_pos) return 0;
	char ch = kbd_buffer[kbd_read_pos];
	kbd_read_pos = (kbd_read_pos + 1) % KBD_BUFFER_SIZE;
	return ch;
}

// O ponto de entrada que o sysinit.c chama é "main"
int main(void)
{
	ScreenInitialise();
	ScreenClear();

	if (!USPiInitialize())
	{
		ScreenWriteString("Falha ao inicializar USPi!");
		while (1);
	}

	ScreenWriteString("USPi Inicializado. Aguardando teclado...\n");

	while (!USPiKeyboardAvailable())
	{
		// Loop de espera
	}

	USPiKeyboardRegisterKeyPressedHandler(KeyPressedHandler);

	ScreenClear();
	ScreenWriteString("Teclado conectado! Pode comecar a digitar...\n\n");

	while (1)
	{
		char ch = GetChar();
		if (ch != 0)
		{
			char str[2] = {ch, '\0'};
			ScreenWriteString(str);
		}
	}
	
	return 0;
}