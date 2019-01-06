/*
 * Saint Petersburg Electrotechnical University, Saint Petersburg, Russia
 * Faculty of Radio Engineering
 * Department of Theoretical Fundamentals of Radio Engineering
 * Vyacheslav P. Klimentyev, 2018
 */

#ifndef _SCMA_LOG_MPA_H_
#define _SCMA_LOG_MPA_H_

#include <math.h>
#include <stddef.h>

#include "scma_parameters.h"

struct complex_double
{
    double real;
    double imag;
};

typedef struct complex_double complex_double_t;

void scmadec(complex_double_t const y[K][N], complex_double_t const cb[K][M][V], complex_double_t const h[K][V][N], double N0, int Niter, double LLR[B][N]);

static inline complex_double_t complex_add(complex_double_t a, complex_double_t b)
{
    complex_double_t c;

    c.real = a.real + b.real;
    c.imag = a.imag + b.imag;

    return c;
}

static inline complex_double_t complex_subst(complex_double_t a, complex_double_t b)
{
    complex_double_t c;

    c.real = a.real - b.real;
    c.imag = a.imag - b.imag;

    return c;
}

static inline complex_double_t complex_mult(complex_double_t a, complex_double_t b)
{
    complex_double_t c;

    c.real = a.real*b.real - a.imag*b.imag;
    c.imag = a.imag*b.real + a.real*b.imag;

    return c;
}

static inline double max_element(double const *ar, size_t size)
{
    double max = ar[0];

    for (int i = 1; i < size; i++)
    {
        if (ar[i] > max)
        {
            max = ar[i];
        }
    }

    return max;
}

static inline double log_sum_exp(double const *x, size_t size)
{
    double xm = max_element(x, size);

    double sum = 0;
    double x_arg = 0;

    #pragma omp simd reduction(+:sum) private(x_arg)
    for (int i = 0; i < size; i++)
    {
        x_arg = x[i] - xm;
        sum += exp(x_arg);
    }

    double log_sum = xm + log(sum);

    return log_sum;
}

#endif // _SCMA_LOG_MPA_H_
