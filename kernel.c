/**
 * kernel.c (Versão Definitiva Final)
 *
 * Corrige a ordem dos includes para resolver todos os erros de compilação.
 */

// A ORDEM DESTES INCLUDES É A SOLUÇÃO PARA O ERRO DE COMPILAÇÃO.
#include "uspios.h"         // 1. ESTE ARQUIVO DEVE SER O PRIMEIRO. Ele define 'u8'.
#include "uspi.h"           // 2. Este arquivo usa 'u8', então deve vir depois.
#include "rpi-mailbox.h"    // 3. O resto dos arquivos.
#include "rpi-systimer.h"
#include <string.h>

// Protótipos
void Log(const char *pSource, int nSeverity, const char *pMessage, ...);
void uspi_assertion_failed(const char *pExpr, const char *pFile, int nLine);
extern const unsigned char _font[];

// --- Framebuffer e Funções de Desenho ---
static volatile unsigned int *framebuffer;
static unsigned int screen_width, screen_height, cursor_x = 0, cursor_y = 0;

void DrawPixel(int x, int y, unsigned int color) {
    if (x < screen_width && y < screen_height) framebuffer[y * screen_width + x] = color;
}

void Print(const char *str) {
    for (int i = 0; str[i]; i++) {
        if (str[i] == '\n') { cursor_x = 0; cursor_y += 16; }
        else {
            for (int j = 0; j < 16; j++) for (int k = 0; k < 8; k++)
                if ((_font[(str[i] * 16) + j] >> k) & 1) DrawPixel(cursor_x + (8-k), cursor_y + j, 0xFFFFFFFF);
            cursor_x += 8;
            if (cursor_x > screen_width - 8) { cursor_x = 0; cursor_y += 16; }
        }
    }
}

void ClearScreen(unsigned int color) {
    for (unsigned int i = 0; i < screen_width * screen_height; i++) framebuffer[i] = color;
    cursor_x = 0; cursor_y = 0;
}

int InitializeFrameBuffer(void) {
    unsigned int mail[32] __attribute__((aligned(16)));
    mail[0] = 8*4; mail[1] = 0; mail[2] = 0x40003; mail[3] = 8; mail[4] = 0; mail[5] = 0; mail[6] = 0; mail[7] = 0;
    MailboxWrite(8, (unsigned int)mail); MailboxRead(8);
    screen_width = mail[5]; screen_height = mail[6];
    mail[0] = 20*4; mail[1] = 0; mail[2] = 0x48003; mail[3] = 8; mail[4] = 8; mail[5] = screen_width; mail[6] = screen_height;
    mail[7] = 0x48004; mail[8] = 8; mail[9] = 8; mail[10] = screen_width; mail[11] = screen_height;
    mail[12] = 0x48005; mail[13] = 4; mail[14] = 4; mail[15] = 32; mail[16] = 0x40001; mail[17] = 8; mail[18] = 4; mail[19] = 0; mail[20] = 0; mail[21] = 0;
    MailboxWrite(8, (unsigned int)mail); MailboxRead(8);
    if (mail[19] == 0) return -1;
    framebuffer = (unsigned int *)((mail[19] & 0x3FFFFFFF) | 0x40000000);
    return 0;
}

// --- Lógica do Teclado ---
#define KBD_BUFFER_SIZE 256
static char kbd_buffer[KBD_BUFFER_SIZE];
static volatile unsigned int kbd_write_pos = 0, kbd_read_pos = 0;

void KeyPressedHandler(const char *pString) {
    kbd_buffer[kbd_write_pos] = pString[0];
    kbd_write_pos = (kbd_write_pos + 1) % KBD_BUFFER_SIZE;
}

char GetChar(void) {
    if (kbd_read_pos == kbd_write_pos) return 0;
    char ch = kbd_buffer[kbd_read_pos];
    kbd_read_pos = (kbd_read_pos + 1) % KBD_BUFFER_SIZE;
    return ch;
}

// --- Ponto de Entrada Principal ---
void kernel_main(void) {
    if (InitializeFrameBuffer() != 0) while(1);
    if (!USPiInitialize()) {
        ClearScreen(0xFFFF0000); Print("Falha ao inicializar USPi!"); while(1);
    }
    
    ClearScreen(0xFF00008B);
    Print("USPi Inicializado. Aguardando teclado...\n");

    while (!USPiKeyboardAvailable()) {
        USPiPeriodicWork();
    }
    
    USPiKeyboardRegisterKeyPressedHandler(KeyPressedHandler);
    ClearScreen(0xFF00008B);
    Print("Teclado conectado! Pode comecar a digitar...\n\n");

    while (1) {
        USPiPeriodicWork();

        char ch = GetChar();
        if (ch != 0) {
            char str[2] = {ch, '\0'};
            Print(str);
        }
    }
}

// --- Funções de suporte exigidas pela USPi ---
void MsDelay(unsigned nMilliseconds) {
    u64 nStart = RPi_GetSystemTimer()->counter_lo;
    while (RPi_GetSystemTimer()->counter_lo - nStart < nMilliseconds * 1000);
}

void uspi_assertion_failed(const char *pExpr, const char *pFile, int nLine) {
    ClearScreen(0xFFFF0000); Print("USPI ASSERTION FAILED\n"); Print(pExpr); while(1);
}

void Log(const char *pSource, int nSeverity, const char *pMessage, ...) {
    // Ignorar logs
}