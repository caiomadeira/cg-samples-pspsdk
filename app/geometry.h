#ifndef GEOMETRY_H
#define GEOMETRY_H

#include "common.h"

typedef struct {
    float x, y, z;
} Point;

typedef struct {
    Point coords[3];
    float CurveTotalLength;
    unsigned int color;
} Bezier;

typedef struct {
    unsigned int color;
    float x, y, z;
} ParametricCurve;


void initBezier(Bezier *b);
void initBezierWithPoints(Bezier* b, Point p0, Point p1, Point p2);
void initBezierWithArray(Bezier *b, Point v[]);
Point calculateBezier(const Bezier* b, double t);
Point getPointControllerBezier(Bezier* b, int i);
void calculateCurveLengthBezier(Bezier *b);
void drawBezier(const Bezier *b, int resolution, const char* type);


void SetPoint(Point *p, float x, float y, float z);

typedef struct {
    Point* vertex;
    int count;
    int capacity;
    Point min, max;
    unsigned int color;
} Polygon;

typedef struct {
    int p0, p1, p2;
} CurveIndices;

// "Construtores" e Memória
void initPolygon(Polygon *p);
void freePolygon(Polygon *p); // Novo! Em C, temos que liberar a memória manualmente.

// Manipulação
void Polygon_insertVertex(Polygon *p, Point pt);
void Polygon_insertVertexAt(Polygon *p, Point pt, int position);
Point Polygon_getVertex(const Polygon *p, int n);
unsigned long Polygon_getNVertex(const Polygon *p);
void Polygon_changeVertex(Polygon *p, int i, Point pt);

// Limites
void Polygon_updateLimits(Polygon *p);
void Polygon_getLimits(const Polygon *p, Point *min, Point *max);

// Leitura de Arquivo
void Polygon_readFromFile(Polygon *p, const char *nome);

// Desenho (PSP GPU)
void Polygon_fillPolygon(const Polygon *p);    // Antigo pintaPoligono
void Polygon_drawPolygon(const Polygon *p);    // Antigo desenhaPoligono (Linhas)
void Polygon_drawVertices(const Polygon *p);   // Antigo desenhaVertices (Pontos)
void Polygon_drawEdge(const Polygon *p, int n);
int loadScene(const char* filename, Polygon* polygon, CurveIndices* curves, int maxCurves);

#endif