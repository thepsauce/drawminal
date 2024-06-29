#include "matrix.h"

#include <math.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

Matrix *matCreate(unsigned short rows, unsigned short cols) {
	Matrix *m = malloc(sizeof(Matrix));
	m->rows = rows;
	m->cols = cols;
	m->elements = malloc(sizeof(float) * rows * cols);
	return m;
}

Matrix *matAdd(Matrix *m1, Matrix *m2) {
	if (m1->rows != m2->rows || m1->cols != m2->cols) {
		printf("Error: (Matrix Addition) Dimensions don't match\n");
		return NULL;
	}

	Matrix *result = matCreate(m1->rows, m1->cols);

	for (int i = 0; i < m1->rows; i++) {
		for (int j = 0; j < m1->cols; j++) {
			result->elements[i * m1->cols + j] =
				m1->elements[i * m1->cols + j] + m2->elements[i * m1->cols + j];
		}
	}

	return result;
}

Matrix *matMultiply(Matrix *m1, Matrix *m2) {
	if (m1->cols != m2->rows) {
		printf("Error: (Matrix multiply) Incompatible Matrices\n");
		return NULL;
	}

	Matrix *result = matCreate(m1->rows, m2->cols);

	for (int i = 0; i < m1->rows; i++) {
		for (int j = 0; j < m2->cols; j++) {
			float sum = 0;
			for (int k = 0; k < m1->cols; k++) {
				sum += m1->elements[i * m1->cols + k] *
					   m2->elements[k * m2->cols + j];
			}
			result->elements[i * result->cols + j] = sum;
		}
	}

	return result;
}

Matrix *getRow(Matrix *m, unsigned short row) {
	if (row >= m->rows) {
		printf("Error: (Matrix getRow) Row index out of bounds.\n");
		return NULL;
	}

	Matrix *rowMatrix = matCreate(1, m->cols);
	if (!rowMatrix) {
		printf("Error: (Matrix getRow) Memory allocation failed.\n");
		return NULL;
	}

	for (size_t j = 0; j < m->cols; j++) {
		rowMatrix->elements[j] = m->elements[row * m->cols + j];
	}

	return rowMatrix;
}

Matrix *getColumn(Matrix *m, unsigned short col) {
	if (col >= m->cols) {
		printf("Error: (Matrix getColumn) Column index out of bounds.\n");
		return NULL;
	}

	Matrix *columnMatrix = matCreate(m->rows, 1);
	if (!columnMatrix) {
		printf("Error: (Matrix getColumn) Memory allocation failed.\n");
		return NULL;
	}

	for (size_t i = 0; i < m->rows; i++) {
		columnMatrix->elements[i] = m->elements[i * m->cols + col];
	}

	return columnMatrix;
}

Matrix *matMinor(Matrix *m, unsigned short row, unsigned short col) {
	Matrix *minor = matCreate(m->rows - 1, m->cols - 1);
	for (size_t i = 0, j = 0; i < m->rows * m->cols; i++) {
		// Check against the given row
		if (i >= (size_t) ((row - 1) * m->cols) && i < (size_t) (row * m->cols)) {
			continue;
		}
		// Check against the given columnMatrix
		if ((i - ELE_POS(m, row, col)) % m->cols == 0) {
			continue;
		}
		minor->elements[j] = m->elements[i];
		j++;
	}
	return minor;
}

float Determinant(Matrix *m) {
	if (m->rows != m->cols) {
		printf(
			"Error: (Matrix matDeterminant) Only square matrices can have "
			"determinants\n");
		return NAN;
	}
	if (m->rows == 1) {
		return m->elements[0];
	}
	if (m->rows == 2) {
		return (
			m->elements[0] * m->elements[3] - m->elements[1] * m->elements[2]);
	}
	if (m->rows > 2) {
		float det = 0;
		for (size_t i = 1; i <= m->cols; i++) {
			Matrix *minor = matMinor(m, 1, i);
			det += m->elements[i - 1] * pow(-1, i + 1) * Determinant(minor);
		}
		return det;
	}
	return NAN;
}

Matrix *matDiag(unsigned short n, ...) {
	Matrix *m = matCreate(n, n);
	va_list args;
	va_start(args, n);
	for (size_t i = 1; i <= n; i++) {
		m->elements[ELE_POS(m, i, i)] = va_arg(args, double);
	}
	va_end(args);
	return m;
}

void matFill(Matrix *m, float x) {
	if (m == NULL) {
		printf("Error: (Matrix matFill) Matrix pointer points to NULL");
	}
	for (size_t i = 0; i < m->rows * m->cols; i++) {
		m->elements[i] = x;
	}
}

Matrix *CreateRotatationMatrix(float theta) {
	Matrix *r = matCreate(2, 2);
	float cos_t = cos(theta);
	float sin_t = sin(theta);
	r->elements[0] = cos_t;
	r->elements[1] = -1 * sin_t;
	r->elements[2] = sin_t;
	r->elements[3] = cos_t;
	return r;
}

Vec2 *vec2Create(unsigned short x, unsigned short y) {
	Vec2 *v = malloc(sizeof(Vec2));
	v->x = x;
	v->y = y;
	return v;
}

Vec2 vec2Transform(Matrix *transformationMatrix, Vec2 v) {
	float *e = transformationMatrix->elements;
	float nx = e[0] * v.x + e[1] * v.y;
	float ny = e[2] * v.x + e[3] * v.y;
	return (Vec2){nx, ny};
}

Vec2 vec2Floor(Vec2 v) { return (Vec2){floor(v.x), floor(v.y)}; }

Matrix *rotateCoordMatrix(Matrix *m, Vec2 axis, float theta) {
	Matrix *new_m = matCreate(m->rows, m->cols);
	matFill(new_m, 0);
	Matrix *rm = CreateRotatationMatrix(theta);
	for (size_t row = 1; row <= m->rows; row++) {
		for (size_t col = 1; col <= m->cols; col++) {
			Vec2 r_pos = (Vec2){row - 1 - axis.x, col - 1 - axis.y};
			Vec2 t_pos = vec2Transform(rm, r_pos);
			int tpx = (int)round(t_pos.x) + 2;
			int tpy = (int)round(t_pos.y) + 2;
			if (1 <= tpx && tpx <= m->cols && 1 <= tpy && tpy <= m->rows) {
				new_m->elements[ELE_POS(new_m, tpx, tpy)] =
					m->elements[ELE_POS(m, row, col)];
			}
		}
	}
	return new_m;
}
