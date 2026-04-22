#include "geometry.h"
#include "common.h"
#include <malloc.h>

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

    if (t < 0.0) t = 0.0;
    if (t > 1.0) t = 1.0;

    double u = 1.0 - t;
    double tt = t * t;
    double uu = u * u;
    double u2t = 2.0 * u * t;

    Point p0 = b->coords[0];
    Point p1 = b->coords[1];
    Point p2 = b->coords[2];

    p.x = (uu * p0.x) + (u2t * p1.x) + (tt * p2.x);
    p.y = (uu * p0.y) + (u2t * p1.y) + (tt * p2.y);
    p.z = (uu * p0.z) + (u2t * p1.z) + (tt * p2.z);

    return p;
}

void drawBezier(const Bezier *b, int resolution, const char* type) {
    //int qtdPoints = 30;

    static ParametricCurve __attribute__((aligned(16))) line[30];
    double t = 0.0;
    double step = 1.0 / (resolution - 1);

    for(int i = 0; i < resolution; i++) {
        Point p = calculateBezier(b, t);

        line[i].color = b->color;
        line[i].x = p.x;
        line[i].y = p.y;
        line[i].z = p.z;

        t += step;
    }

    sceKernelDcacheWritebackAll();
    if (strcmp(type, "3D") == 0) {
        sceGuEnable(GU_DEPTH_TEST);
        sceGuDrawArray(GU_LINE_STRIP, 
        GU_COLOR_8888 | GU_VERTEX_32BITF | GU_TRANSFORM_3D, 
        resolution, 0, line);
    } else if (strcmp(type, "2D") == 0) {
        sceGuDisable(GU_DEPTH_TEST);
        sceGuDrawArray(GU_LINE_STRIP, 
            GU_COLOR_8888 | GU_VERTEX_32BITF | GU_TRANSFORM_2D, 
            resolution, 0, line);
    } else { pspDebugScreenInit(); print("Error.\n"); }
}

void initPolygon(Polygon *p) {
    p->count = 0;
    p->capacity = 10; // Começamos com espaço para 10 vértices
    p->vertex = (Point*)malloc(p->capacity * sizeof(Point));
    p->color = 0xFFFFFFFF; // Branco por padrão
}

void freePolygon(Polygon *p) {
    if (p->vertex) {
        free(p->vertex);
        p->vertex = NULL;
    }
    p->count = 0;
    p->capacity = 0;
}

void Polygon_insertVertex(Polygon *p, Point pt) {
    // Se encheu, dobramos a capacidade (igual o std::vector faz por baixo dos panos)
    if (p->count >= p->capacity) {
        p->capacity *= 2;
        p->vertex = (Point*)realloc(p->vertex, p->capacity * sizeof(Point));
    }
    p->vertex[p->count] = pt;
    p->count++;
}

// -------------------------------------------------------------
// LEITURA DE ARQUIVO (ifstream vira FILE*)
// -------------------------------------------------------------

void Polygon_readFromFile(Polygon *p, const char *nome) {
    FILE *input = fopen(nome, "r");
    if (!input) {
        printf("Erro ao abrir %s.\n", nome);
        return;
    }

    int qtdVertices;
    if (fscanf(input, "%d", &qtdVertices) != 1) {
        fclose(input);
        return;
    }

    for (int i = 0; i < qtdVertices; i++) {
        float x, y;
        // No C, usamos %f para ler floats de um arquivo de texto
        if (fscanf(input, "%f %f", &x, &y) == 2) {
            Point pt; 
            SetPoint(&pt, x, y, 0.0f);
            Polygon_insertVertex(p, pt);
        } else {
            break; // Fim do arquivo ou erro de formatação
        }
    }
    fclose(input);
}

// -------------------------------------------------------------
// DESENHO NO PSP (A Conversão do OpenGL)
// -------------------------------------------------------------

void Polygon_fillPolygon(const Polygon *p) {
    if (p->count < 3) return; // Não dá pra preencher menos que um triângulo

    // Alocamos um array temporário alinhado (16 bytes) para a GPU ler
    ParametricCurve *gpu_vertices = (ParametricCurve*)memalign(16, p->count * sizeof(ParametricCurve));

    for (int i = 0; i < p->count; i++) {
        gpu_vertices[i].color = p->color;
        gpu_vertices[i].x = p->vertex[i].x;
        gpu_vertices[i].y = p->vertex[i].y;
        gpu_vertices[i].z = p->vertex[i].z;
    }

    sceKernelDcacheWritebackAll(); // Limpa o cache L1!

    // GL_POLYGON no OpenGL equivale ao GU_TRIANGLE_FAN no PSP
    sceGuDrawArray(GU_TRIANGLE_FAN, GU_COLOR_8888 | GU_VERTEX_32BITF | GU_TRANSFORM_2D, p->count, 0, gpu_vertices);

    free(gpu_vertices); // Libera a memória após desenhar
}

void Polygon_drawPolygon(const Polygon *p) {
    if (p->count < 2) return;

    // O PSP NÃO TEM "GL_LINE_LOOP". 
    // Para fechar o polígono, precisamos de um vértice a mais repetindo o primeiro.
    int gpu_count = p->count + 1;
    ParametricCurve *gpu_vertices = (ParametricCurve*)memalign(16, gpu_count * sizeof(ParametricCurve));

    for (int i = 0; i < p->count; i++) {
        gpu_vertices[i].color = p->color;
        gpu_vertices[i].x = p->vertex[i].x;
        gpu_vertices[i].y = p->vertex[i].y;
        gpu_vertices[i].z = p->vertex[i].z;
    }
    
    // Copia o primeiro vértice para o final para fechar o "Loop"
    gpu_vertices[p->count] = gpu_vertices[0];

    sceKernelDcacheWritebackAll();

    // GL_LINE_LOOP no PC vira GU_LINE_STRIP no PSP (com o último ponto repetido)
    sceGuDrawArray(GU_LINE_STRIP, GU_COLOR_8888 | GU_VERTEX_32BITF | GU_TRANSFORM_2D, gpu_count, 0, gpu_vertices);

    free(gpu_vertices);
}

// Retorna um vértice específico pelo seu índice
Point Polygon_getVertex(const Polygon *p, int n) {
    // Trava de segurança: se pedirem um índice fora do limite, retorna a origem
    if (n < 0 || n >= p->count) {
        Point origem;
        SetPoint(&origem, 0.0f, 0.0f, 0.0f);
        return origem;
    }
    return p->vertex[n];
}

// Retorna a quantidade total de vértices no polígono
unsigned long Polygon_getNVertex(const Polygon *p) {
    return p->count;
}

void Polygon_drawVertices(const Polygon *p) {
    if (p->count == 0) return;

    ParametricCurve *gpu_vertices = (ParametricCurve*)memalign(16, p->count * sizeof(ParametricCurve));

    for (int i = 0; i < p->count; i++) {
        gpu_vertices[i].color = p->color;
        gpu_vertices[i].x = p->vertex[i].x;
        gpu_vertices[i].y = p->vertex[i].y;
        gpu_vertices[i].z = p->vertex[i].z;
    }

    sceKernelDcacheWritebackAll();

    // GL_POINTS equivale a GU_POINTS
    sceGuDrawArray(GU_POINTS, GU_COLOR_8888 | GU_VERTEX_32BITF | GU_TRANSFORM_2D, p->count, 0, gpu_vertices);

    free(gpu_vertices);
}

int loadScene(const char* filename, Polygon* polygon, CurveIndices* curves, int maxCurves) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        pspDebugScreenInit();
        print("Erro ao abrir o arquivo.\n");
        return 0;
    }

    int numPoints;
    if (fscanf(file, "%d", &numPoints) != 1) {
        fclose(file);
        return 0;
    }

    for (int i = 0; i < numPoints; i++) {
        float x, y;
        fscanf(file, "%f %f", &x, &y);
        Point pt;
        SetPoint(&pt, (x * 30.0f) + 240.0f, (y * 30.0f) + 136.0f, 0.0f);
        Polygon_insertVertex(polygon, pt);
    }

    int numCurves;
    fscanf(file, "%d", &numCurves);

    if (numCurves > maxCurves) {
        numCurves = maxCurves;
    }

    for(int i = 0; i < numCurves; i++) {
        fscanf(file, "%d %d %d", &curves[i].p0, &curves[i].p1, &curves[i].p2);
    }

    fclose(file);
    return numCurves;
}