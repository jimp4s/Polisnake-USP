/**
 * kernel.c
 *
 * Programa bare-metal para Raspberry Pi que inicializa um framebuffer,
 * aguarda por um teclado USB, e ecoa os caracteres digitados na tela.
 *
 * Depende da biblioteca USPi e de funções de hardware para
 * timers, GPIO e mailbox.
 */

// Includes da biblioteca USPi e de hardware
#include "uspi.h"
#include "uspios.h"
#include "uspikeyboard.h"
#include "rpi-mailbox.h"
#include "rpi-systimer.h"
#include <string.h>

// Protótipos de funções de hardware (normalmente em outros arquivos)
// Para simplificar, declaramos extern aqui. Elas vêm do ambiente da USPi.
void Log(const char *pSource, int nSeverity, const char *pMessage, ...);
void uspi_assertion_failed (const char *pExpr, const char *pFile, int nLine);

// --- Configuração do Framebuffer e Desenho Básico ---

// Variáveis globais para o framebuffer
static volatile unsigned int *framebuffer;
static unsigned int screen_width;
static unsigned int screen_height;
static unsigned int screen_pitch;
static unsigned int cursor_x = 0;
static unsigned int cursor_y = 0;

// Fonte 8x16 simples (só alguns caracteres para o exemplo)
extern const unsigned char _font[];

// Função para inicializar o framebuffer via Mailbox
int InitializeFrameBuffer(void) {
    unsigned int mail[32] __attribute__((aligned(16)));

    // Obter resolução física (real)
    mail[0] = 8 * 4;
    mail[1] = 0; // Request
    mail[2] = 0x40003; // Tag: Get physical width/height
    mail[3] = 8;
    mail[4] = 0;
    mail[5] = 0; // Largura
    mail[6] = 0; // Altura
    mail[7] = 0; // Fim da lista
    MailboxWrite(8, (unsigned int)mail);
    MailboxRead(8);
    
    screen_width = mail[5];
    screen_height = mail[6];

    // Configurar framebuffer virtual
    mail[0] = 20 * 4;
    mail[1] = 0; // Request
    mail[2] = 0x48003; // Tag: Set physical width/height
    mail[3] = 8;
    mail[4] = 8;
    mail[5] = screen_width;
    mail[6] = screen_height;
    mail[7] = 0x48004; // Tag: Set virtual width/height
    mail[8] = 8;
    mail[9] = 8;
    mail[10] = screen_width;
    mail[11] = screen_height;
    mail[12] = 0x48005; // Tag: Set depth
    mail[13] = 4;
    mail[14] = 4;
    mail[15] = 32; // 32 bits por pixel
    mail[16] = 0x40001; // Tag: Allocate buffer
    mail[17] = 8;
    mail[18] = 4;
    mail[19] = 0; // Response: pointer
    mail[20] = 0; // Response: size
    mail[21] = 0; // Fim da lista

    MailboxWrite(8, (unsigned int)mail);
    MailboxRead(8);

    if (mail[19] == 0) return -1;

    // Converte endereço do barramento da GPU para endereço físico do ARM
    framebuffer = (unsigned int *)((mail[19] & 0x3FFFFFFF) | 0x40000000);
    screen_pitch = screen_width * 4; // 4 bytes por pixel (32 bits)
    
    return 0;
}

void DrawPixel(int x, int y, unsigned int color) {
    if (x >= screen_width || y >= screen_height || x < 0 || y < 0) return;
    framebuffer[y * screen_width + x] = color;
}

void DrawChar(unsigned char c, int x, int y, unsigned int color) {
    int i, j;
    for (j = 0; j < 16; j++) {
        for (i = 0; i < 8; i++) {
            if ((_font[(c * 16) + j] >> i) & 1) {
                DrawPixel(x + (8-i), y + j, color);
            }
        }
    }
}

void Print(const char *str) {
    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] == '\n') {
            cursor_x = 0;
            cursor_y += 16; // Próxima linha
        } else {
            DrawChar(str[i], cursor_x, cursor_y, 0xFFFFFFFF); // Branco
            cursor_x += 8;
            if (cursor_x > screen_width - 8) {
                cursor_x = 0;
                cursor_y += 16;
            }
        }
    }
}

void ClearScreen(unsigned int color) {
    for (unsigned int y = 0; y < screen_height; y++) {
        for (unsigned int x = 0; x < screen_width; x++) {
            DrawPixel(x, y, color);
        }
    }
    cursor_x = 0;
    cursor_y = 0;
}

// --- Ponto de Entrada Principal ---

void kernel_main(void) {
    // 1. Inicializar a tela
    if (InitializeFrameBuffer() != 0) {
        // Se falhar, ficamos presos aqui. O LED ACT deve piscar um padrão de erro.
        while(1);
    }
    ClearScreen(0xFF0000AA); // Fundo azul escuro
    Print("Inicializacao do Framebuffer OK.\n");

    // 2. Inicializar a biblioteca USPi
    // A flag bStayInCores_ IRQ/FIQ handler é importante para bare-metal
    if (!USPiInitialise(bStayInCores)) {
        Print("Falha ao inicializar USPi.\n");
        while(1);
    }
    Print("USPi Inicializado. Aguardando teclado...\n");
    
    // 3. Loop de verificação do teclado
    while (USPiKeyboardAvailable() == 0) {
        // Função necessária para USPi processar eventos em modo polling
        USPiPeriodicWork(); 
    }
    ClearScreen(0xFF0000AA);
    Print("Teclado conectado! Comece a digitar...\n\n");

    // 4. Loop principal de eco do teclado
    char ch;
    while (1) {
        // Processa eventos USB
        USPiPeriodicWork();
        
        // Pega o próximo caractere do buffer do teclado
        ch = USPiKeyboardGetChar();

        if (ch != 0) { // Se um caractere foi pressionado
            char str[2] = {ch, '\0'};
            Print(str);
        }
    }
}

// --- Funções de suporte exigidas pela USPi ---
// Usamos as implementações padrão que vêm com a biblioteca

void MsDelay (unsigned nMilliseconds) {
    u64 nStart = RPi_GetSystemTimer()->counter_lo;
    while (RPi_GetSystemTimer()->counter_lo - nStart < nMilliseconds * 1000);
}

void uspi_assertion_failed (const char *pExpr, const char *pFile, int nLine) {
    // Em caso de erro crítico da USPi, imprime e para
    ClearScreen(0xFFFF0000); // Tela vermelha
    Print("USPI ASSERTION FAILED\n");
    Print(pExpr);
    while (1);
}

// Implementação simples de log para USPi (não faz nada)
void Log(const char *pSource, int nSeverity, const char *pMessage, ...) {
    // Ignorar logs para este exemplo simples
}