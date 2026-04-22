#include "geometry.h"

PSP_MODULE_INFO("BezierPSP", 0, 1, 0);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);

#define RED 0xFF0000FF // ALPHA = FF, BLUE = 00, GREEN = 00, RED = FF
#define DARK_BLUE 0xFF880000
#define Vector3 ScePspFVector3

#define GU_COLOR(r, g, b, a) (((a) << 24) | ((b) << 16) | ((g) << 8 ) | (r))

unsigned int __attribute__((aligned(16))) list[262144];

void init_graphics() {
    sceGuInit();
    sceGuStart(GU_DIRECT, list);

    sceGuDrawBuffer(GU_PSM_8888, (void*)0, 512);
    sceGuDispBuffer(480, 272, (void*)0x88000, 512);
    sceGuDepthBuffer((void*)0x110000, 512);

    sceGuEnable(GU_DEPTH_TEST);
    sceGuDepthFunc(GU_GEQUAL);

    sceGuOffset(2048 - (480/2), 2048 - (272/2));
    sceGuViewport(2048, 2048, 480, 272);

    sceGuEnable(GU_SCISSOR_TEST);
    sceGuScissor(0, 0, 480, 272);

    sceGuFinish();
    sceGuSync(0,0);
    sceDisplayWaitVblankStart();
    sceGuDisplay(GU_TRUE);
}

void init() {
    sceGuClearColor(DARK_BLUE);
}


int main() {
    SetupCallbacks(); 

    // Bezier curve;
    // curve.color = RED;

    // Point p0, p1, p2;
    // SetPoint(&p0, 50.0f,  200.0f, 0.0f);
    // SetPoint(&p1, 240.0f,  50.0f, 0.0f);
    // SetPoint(&p2, 430.0f, 200.0f, 0.0f);

    // curve.coords[0] = p0;
    // curve.coords[1] = p1;
    // curve.coords[2] = p2;


    init_graphics();
    init();

    Polygon controlPoints;
    initPolygon(&controlPoints);
    CurveIndices curves[50];

    int totalCurves = loadScene("curvas.txt", &controlPoints, curves, 50);
    if (totalCurves == 0) {
        sceGuClearColor(0xFF0000FF); // Fundo Vermelho = Erro no arquivo
    }
    Bezier tempCurve;

    while(1) {
        sceGuStart(GU_DIRECT, list);
        sceGuClear(GU_COLOR_BUFFER_BIT);
        
        for(int i = 0; i < totalCurves; i++) {
            unsigned int r = (40 + (i * 25)) % 256;
            unsigned int g = 150;
            unsigned int b = (255 - (i * 15)) % 256;

            tempCurve.color = GU_COLOR(r, b, g, 255);
            Point p0 = Polygon_getVertex(&controlPoints, curves[i].p0);
            Point p1 = Polygon_getVertex(&controlPoints, curves[i].p1);
            Point p2 = Polygon_getVertex(&controlPoints, curves[i].p2);
            
            tempCurve.coords[0] = p0;
            tempCurve.coords[1] = p1;
            tempCurve.coords[2] = p2;

            drawBezier(&tempCurve, 30, "2D");
        }

        sceGuFinish();
        sceGuSync(0,0);
        sceDisplayWaitVblankStart();
        sceGuSwapBuffers();
    }

    sceGuTerm();
    sceKernelExitGame();
    return 0;
}