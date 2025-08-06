/**
 * kernel.c (v8 - Final e Definitivo)
 * Usa a API de alto nível da sua biblioteca e a ordem de includes correta.
 */

// A ORDEM CORRETA QUE RESOLVE TODOS OS ERROS
#include "uspios.h"
#include "uspi.h"
#include "uspienv.h" // Header principal do seu ambiente
#include "screen.h"  // Header para as funções de tela

// --- Buffer do Teclado ---
#define KBD_BUFFER_SIZE 256
static char kbd_buffer[KBD_BUFFER_SIZE];
static volatile unsigned int kbd_write_pos = 0, kbd_read_pos = 0;

// Handler que a USPi chama quando uma tecla é pressionada
void KeyPressedHandler(const char *pString)
{
    // Apenas armazena o primeiro caractere da string (ignora Shift, etc.)
	if (*pString != '\0')
	{
		kbd_buffer[kbd_write_pos] = *pString;
		kbd_write_pos = (kbd_write_pos + 1) % KBD_BUFFER_SIZE;
	}
}

// Função para nosso loop principal pegar um caractere do buffer
char GetChar(void)
{
	if (kbd_read_pos == kbd_write_pos)
	{
		return 0; // Buffer vazio
	}

	char ch = kbd_buffer[kbd_read_pos];
	kbd_read_pos = (kbd_read_pos + 1) % KBD_BUFFER_SIZE;
	return ch;
}

// --- Ponto de Entrada Principal ---
void kernel_main(void)
{
	// A sua biblioteca já tem uma função para inicializar a tela!
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
		// Na sua versão da biblioteca, USPiInitialize inicia um timer
		// que chama o trabalho periódico, então não precisamos chamá-lo em loop.
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
}