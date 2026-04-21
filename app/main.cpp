#include "common.h"

PSP_MODULE_INFO("BezierPSP", 0, 1, 0);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);

// Display list (Fila de comandos da GPU)
unsigned int __attribute__((aligned(16))) list[262144];

// Função para iniciar a Placa de Vídeo do PSP
void initGraphics() {
    sceGuInit();
    sceGuStart(GU_DIRECT, list);
    
    sceGuDrawBuffer(GU_PSM_8888, (void*)0, 512);
    sceGuDispBuffer(480, 272, (void*)0x88000, 512);
    sceGuDepthBuffer((void*)0x110000, 512);
    
    sceGuOffset(2048 - (480/2), 2048 - (272/2));
    sceGuViewport(2048, 2048, 480, 272);
    sceGuDepthRange(65535, 0);
    
    sceGuEnable(GU_SCISSOR_TEST);
    sceGuScissor(0, 0, 480, 272);
    sceGuEnable(GU_DEPTH_TEST);
    sceGuDepthFunc(GU_GEQUAL);

    sceGuFinish();
    sceGuSync(0,0);
    sceDisplayWaitVblankStart();
    sceGuDisplay(GU_TRUE);
}

int main() {
    SetupCallbacks(); // Inicia os botões para poder sair do jogo
    initGraphics();   // Liga o ecrã

    // 1. Configurar a nossa Curva de Bézier
    Bezier minhaCurva;
    minhaCurva.color = 0xFF00FFFF; // Amarelo no formato ABGR

    // Definir 3 pontos de controlo no espaço 3D
    Point p0, p1, p2;
    SetPoint(&p0, -2.0f, -1.0f, 0.0f); // Ponto inicial (Esquerda, baixo)
    SetPoint(&p1,  0.0f,  3.0f, 0.0f); // Ponto de controlo (Centro, alto - puxa a curva)
    SetPoint(&p2,  2.0f, -1.0f, 0.0f); // Ponto final (Direita, baixo)

    // Associar os pontos à curva
    minhaCurva.coords[0] = p0;
    minhaCurva.coords[1] = p1;
    minhaCurva.coords[2] = p2;

    float angle = 0.0f;

    // 2. Loop Principal (Game Loop)
    while(1) {
        sceGuStart(GU_DIRECT, list);

        // Fundo escuro
        sceGuClearColor(0xFF222222);
        sceGuClearDepth(0);
        sceGuClear(GU_COLOR_BUFFER_BIT | GU_DEPTH_BUFFER_BIT);

        // --- MATRIZ DE PROJEÇÃO ---
        sceGumMatrixMode(GU_PROJECTION);
        sceGumLoadIdentity();
        sceGumPerspective(75.0f, 16.0f/9.0f, 0.5f, 100.0f);

        // --- MATRIZ DA CÂMARA (View) ---
        sceGumMatrixMode(GU_VIEW);
        sceGumLoadIdentity();

        // --- MATRIZ DO MODELO (Onde a curva vai ficar) ---
        sceGumMatrixMode(GU_MODEL);
        sceGumLoadIdentity();

        // Translação: Afastar a curva 6 unidades para trás para conseguirmos vê-la (Eixo Z = -6)
        // Translação: Baixar a curva 1 unidade (Eixo Y = -1)
        ScePspFVector3 pos = {0.0f, -1.0f, -6.0f};
        sceGumTranslate(&pos);

        // Rotação: Fazer a curva girar lentamente no eixo Y
        sceGumRotateY(angle);

        // 3. DESENHAR A CURVA!
        // Isto chama a função que construímos e preenche a Display List com a linha
        drawBezier(&minhaCurva);

        sceGuFinish(); // Fecha a lista
        sceGuSync(0,0); // GPU executa a lista
        sceDisplayWaitVblankStart(); // Espera pelo ecrã
        sceGuSwapBuffers(); // Mostra a imagem

        angle += 0.02f; // Incrementa a rotação para o próximo frame
    }

    sceGuTerm();
    sceKernelExitGame();
    return 0;
}