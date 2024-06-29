#ifndef MATRIX_H
#define MATRIX_H

#define ELE_POS(m, row, col) (row - 1) * m->cols + col - 1
#define ARRLEN(arr) sizeof(arr) / sizeof(arr[0])

typedef struct matrix {
	unsigned short rows;
	unsigned short cols;
	float *elements;
} Matrix;

typedef struct vec2 {
	float x;
	float y;
} Vec2;

Matrix *matCreate(unsigned short rows, unsigned short cols);
Matrix *matAdd(Matrix *m1, Matrix *m2);
Matrix *matMultiply(Matrix *m1, Matrix *m2);
Matrix *matRow(Matrix *m, unsigned short row);
Matrix *matColumn(Matrix *m, unsigned short col);
Matrix *matMinor(Matrix *m, unsigned short row, unsigned short col);
float Determinant(Matrix *m);
Matrix *matDiag(unsigned short n, ...);
void matFill(Matrix *m, float x);
Matrix *CreateRotatationMatrix(float theta);  // Only 2D

Vec2 *vec2Create(unsigned short x, unsigned short y);
Vec2 vec2Transform(Matrix *transformationMatrix, Vec2 v);
Vec2 vec2Floor(Vec2 v);

Matrix *rotateCoordMatrix(Matrix *m, Vec2 axis, float theta);

#endif
