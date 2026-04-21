#include "common.h"

// callback classico pra sair do jogo apertando o botão home do PSP
int exit_callback(int arg1, int arg2, void *common) { sceKernelExitGame(); return 0; }
int CallbackThread(SceSize args, void *argp) {
    int cbid = sceKernelCreateCallback("Exit Callback", exit_callback, NULL);
    sceKernelRegisterExitCallback(cbid);
    sceKernelSleepThreadCB();
    return 0;
}
void SetupCallbacks(void) {
    int thid = sceKernelCreateThread("update_thread", CallbackThread, 0x11, 0xFA0, 0, 0);
    if(thid >= 0) sceKernelStartThread(thid, 0, 0);
}

// ----------------------------------------------------------

void SetPoint(Point *p, float x, float y, float z) {
    p->x = x;
    p->y = y;
    p->z = z;
}

Point getPointControllerBezier(const Bezier* b, int i) {
    return b->coords[i];
}

Point calculateBezier(const Bezier* b, double t) {
    Point p;

    // Trava de segurança: garante que o 't' não ultrapasse os limites da curva
    if (t < 0.0) t = 0.0;
    if (t > 1.0) t = 1.0;

    // Pré-calcula as partes da equação para otimizar o processamento
    double u = 1.0 - t;
    double tt = t * t;
    double uu = u * u;
    double u2t = 2.0 * u * t;

    // Atalhos para os pontos de controle (deixa o código mais legível)
    Point p0 = b->coords[0];
    Point p1 = b->coords[1];
    Point p2 = b->coords[2];

    // Calcula as posições X, Y e Z aplicando a fórmula
    p.x = (uu * p0.x) + (u2t * p1.x) + (tt * p2.x);
    p.y = (uu * p0.y) + (u2t * p1.y) + (tt * p2.y);
    p.z = (uu * p0.z) + (u2t * p1.z) + (tt * p2.z);

    return p;
}

void drawBezier(const Bezier *b) {
    int qtdPoints = 30;

    // alocar espaço p/ os vertices na RAM
    static VertexCurve __attribute__((aligned(16))) line[30];
    double t = 0.0;
    double step = 1.0 / (qtdPoints - 1);

    for(int i = 0; i < qtdPoints; i++) {
        Point p = calculateBezier(b, t);

        line[i].color = b->color;
        line[i].x = p.x;
        line[i].y = p.y;
        line[i].z = p.z;

        t += step;
    }

    sceGumDrawArray(GU_LINE_STRIP, 
        GU_COLOR_8888 | GU_VERTEX_32BITF | GU_TRANSFORM_3D, 
        qtdPoints, 0, line);
}
