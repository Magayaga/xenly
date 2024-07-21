/*
 * XENLY - high-level and general-purpose programming language
 * created, designed, and developed by Cyril John Magayaga (cjmagayaga957@gmail.com, cyrilmagayaga@proton.me).
 *
 * It is initially written in C programming language.
 *
 * `xenly_numerical_analysis_math.c` is the similar to the `xenly_numerical_analysis_math.jl` in Julia programming language.
 * 
 * It is available for Linux and Windows operating systems.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// GMRES (Generalized Minimal RESidual method)
void xenly_gmres(int n, double A[n][n], double b[n], double x0[n], double tol, int maxiter, double x[n], int *iterations) {
    double Q[n][maxiter + 1], H[maxiter + 1][maxiter];
    double r[n], beta, y[maxiter];
    int i, j;

    // Initialize r, beta, Q
    for (i = 0; i < n; i++) r[i] = b[i] - A[i][0] * x0[0] - A[i][1] * x0[1];
    beta = sqrt(r[0] * r[0] + r[1] * r[1]);
    for (i = 0; i < n; i++) Q[i][0] = r[i] / beta;

    for (j = 0; j < maxiter; j++) {
        double v[n];
        for (i = 0; i < n; i++) v[i] = A[i][0] * Q[0][j] + A[i][1] * Q[1][j];
        for (i = 0; i <= j; i++) {
            H[i][j] = Q[0][i] * v[0] + Q[1][i] * v[1];
            v[0] -= H[i][j] * Q[0][i];
            v[1] -= H[i][j] * Q[1][i];
        }
        H[j+1][j] = sqrt(v[0] * v[0] + v[1] * v[1]);
        if (H[j+1][j] < tol) break;
        for (i = 0; i < n; i++) Q[i][j+1] = v[i] / H[j+1][j];
        for (i = 0; i <= j; i++) y[i] = beta * ((i == j) ? 1 : 0);
        for (i = j; i >= 0; i--) {
            y[i] -= H[i][j] * y[j];
            y[i] /= H[i][i];
        }
        for (i = 0; i < n; i++) x[i] = x0[i] + Q[i][j] * y[j];
        for (i = 0; i < n; i++) r[i] = b[i] - A[i][0] * x[0] - A[i][1] * x[1];
        if (sqrt(r[0] * r[0] + r[1] * r[1]) < tol) {
            *iterations = j;
            return;
        }
    }
    *iterations = maxiter;
}

// Conjugate gradient method
void xenly_conjugate_gradient(int n, double A[n][n], double b[n], double x0[n], double tol, int maxiter, double x[n], int *iterations) {
    double r[n], p[n], Ap[n];
    double alpha, rsold, rsnew;
    int i, iter;

    for (i = 0; i < n; i++) x[i] = x0[i];
    for (i = 0; i < n; i++) r[i] = b[i] - A[i][0] * x0[0] - A[i][1] * x0[1];
    for (i = 0; i < n; i++) p[i] = r[i];
    rsold = r[0] * r[0] + r[1] * r[1];

    for (iter = 0; iter < maxiter; iter++) {
        for (i = 0; i < n; i++) Ap[i] = A[i][0] * p[0] + A[i][1] * p[1];
        alpha = rsold / (p[0] * Ap[0] + p[1] * Ap[1]);
        for (i = 0; i < n; i++) x[i] += alpha * p[i];
        for (i = 0; i < n; i++) r[i] -= alpha * Ap[i];
        rsnew = r[0] * r[0] + r[1] * r[1];
        if (sqrt(rsnew) < tol) {
            *iterations = iter;
            return;
        }
        for (i = 0; i < n; i++) p[i] = r[i] + (rsnew / rsold) * p[i];
        rsold = rsnew;
    }
    *iterations = maxiter;
}

// Gaussian elimination
void xenly_gaussian_elimination(int n, double A[n][n], double b[n], double x[n]) {
    double Ab[n][n+1];
    int i, j, k, maxrow;
    double maxval, factor;

    for (i = 0; i < n; i++) for (j = 0; j < n; j++) Ab[i][j] = A[i][j];
    for (i = 0; i < n; i++) Ab[i][n] = b[i];

    for (i = 0; i < n-1; i++) {
        maxval = fabs(Ab[i][i]);
        maxrow = i;
        for (j = i+1; j < n; j++) {
            if (fabs(Ab[j][i]) > maxval) {
                maxval = fabs(Ab[j][i]);
                maxrow = j;
            }
        }
        
        if (maxrow != i) {
            for (k = 0; k < n+1; k++) {
                double tmp = Ab[i][k];
                Ab[i][k] = Ab[maxrow][k];
                Ab[maxrow][k] = tmp;
            }
        }
        
        for (j = i+1; j < n; j++) {
            factor = Ab[j][i] / Ab[i][i];
            for (k = i; k < n+1; k++) Ab[j][k] -= factor * Ab[i][k];
        }
    }

    for (i = n-1; i >= 0; i--) {
        x[i] = Ab[i][n] / Ab[i][i];
        for (j = i-1; j >= 0; j--) Ab[j][n] -= Ab[j][i] * x[i];
    }
}

// Newton's method
void xenly_newtons_method(void (*f)(double*, double*), void (*df)(double*, double*), double x0[], double tol, int maxiter, double x[], int *iterations) {
    double fx[2], dfx[4], dx[2];
    int i, iter;
    for (i = 0; i < 2; i++) x[i] = x0[i];
    for (iter = 0; iter < maxiter; iter++) {
        f(x, fx);
        if (sqrt(fx[0] * fx[0] + fx[1] * fx[1]) < tol) {
            *iterations = iter;
            return;
        }
        df(x, dfx);
        dx[0] = (fx[0] * dfx[3] - fx[1] * dfx[1]) / (dfx[0] * dfx[3] - dfx[1] * dfx[2]);
        dx[1] = (fx[1] * dfx[0] - fx[0] * dfx[2]) / (dfx[0] * dfx[3] - dfx[1] * dfx[2]);
        for (i = 0; i < 2; i++) x[i] -= dx[i];
    }
    *iterations = maxiter;
}

// Hill Climbing
void xenly_hill_climbing(double (*f)(double*), double x[], double step_size, double tol, int maxiter, double x_out[], double *best_value, int *iterations) {
    double candidate[2], candidate_value;
    int i, iter;

    for (i = 0; i < 2; i++) x_out[i] = x[i];
    *best_value = f(x_out);

    for (iter = 0; iter < maxiter; iter++) {
        for (i = 0; i < 2; i++) candidate[i] = x_out[i] + step_size * (2 * ((double)rand() / RAND_MAX) - 1);
        candidate_value = f(candidate);
        if (candidate_value < *best_value) {
            for (i = 0; i < 2; i++) x_out[i] = candidate[i];
            *best_value = candidate_value;
        }
        
        if (sqrt(candidate_value) < tol) {
            *iterations = iter;
            return;
        }
    }
    *iterations = maxiter;
}

// Bisection method
double xenly_bisection_method(double (*f)(double), double a, double b, double tol, int maxiter, int *iterations) {
    double c;
    int iter;

    for (iter = 0; iter < maxiter; iter++) {
        c = (a + b) / 2;
        if (fabs(f(c)) < tol) {
            *iterations = iter;
            return c;
        }
        
        if (f(a) * f(c) < 0) b = c;
        else a = c;
    }
    *iterations = maxiter;
    return c;
}

// Euler's method
void xenly_eulers_method(double (*f)(double, double), double y0, double t0, double t_end, double h, int *len, double **t_out, double **y_out) {
    int n = (int)((t_end - t0) / h) + 1;
    double *t = (double *)malloc(n * sizeof(double));
    double *y = (double *)malloc(n * sizeof(double));

    int i;
    t[0] = t0;
    y[0] = y0;
    for (i = 1; i < n; i++) {
        t[i] = t[i-1] + h;
        y[i] = y[i-1] + h * f(t[i-1], y[i-1]);
    }
    *len = n;
    *t_out = t;
    *y_out = y;
}

// Example functions
void f(double *x, double *y) {
    y[0] = x[0] * x[0] + x[1] * x[1] - 4;
    y[1] = x[0] * x[0] - x[1] * x[1] - 1;
}

void df(double *x, double (*dy)[2]) {
    dy[0][0] = 2 * x[0];
    dy[0][1] = 2 * x[1];
    dy[1][0] = 2 * x[0];
    dy[1][1] = -2 * x[1];
}

double f_hill_climbing(double x[2]) {
    return (x[0] - 1) * (x[0] - 1) + (x[1] - 1) * (x[1] - 1);
}

double f_bisection(double x) {
    return x * x - 2;
}

double f_euler(double t, double y) {
    return t + y;
}
