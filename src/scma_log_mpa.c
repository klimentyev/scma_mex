/*
 * Saint Petersburg Electrotechnical University, Saint Petersburg, Russia
 * Faculty of Radio Engineering
 * Department of Theoretical Fundamentals of Radio Engineering
 * Vyacheslav P. Klimentyev, 2018
 */

#include "scma_log_mpa.h"
#include <omp.h>

void scmadec(complex_double_t const y[K][N], complex_double_t const cb[K][M][V], complex_double_t const h[K][V][N], double N0, int Niter, double LLR[B][N])
{
    // Factor graph calculation
    int F[K][V]         = {0};
    int ind_df[K][df]   = {0};
    int ind_dv[V][dv]   = {0};

    for (int k = 0; k < K; k++)
    {
        for (int v = 0; v < V; v++)
        {
            for (int m = 0; m < M; m++)
            {
                if ((cb[k][m][v].real != 0.0) || (cb[k][m][v].imag != 0.0))
                {
                    F[k][v] = 1;
                    break;
                }
            }
        }
    }

    for (int k = 0; k < K; k++)
    {
        int idx = 0;
        for (int v = 0; v < V; v++)
        {
            if (F[k][v] == 1)
            {
                ind_df[k][idx] = v;
                idx++;
            }
        }
    }

    for (int v = 0; v < V; v++)
    {
        int idx = 0;
        for (int k = 0; k < K; k++)
        {
            if (F[k][v] == 1)
            {
                ind_dv[v][idx] = k;
                idx++;
            }
        }
    }

    #pragma omp parallel for
    for (int n = 0; n < N; n++)
    {
        // Step 1: Initial calculations

        double f[K][M][M][M] = {0};

        for (int k = 0; k < K; k++)
        {
            for(int m1 = 0; m1 < M; m1++)
            {
                for (int m2 = 0; m2 < M; m2++)
                {
                    for (int m3 = 0; m3 < M; m3++)
                    {
                        complex_double_t sum;
                        sum = complex_add(complex_add(complex_mult(cb[k][m1][ind_df[k][0]], h[k][ind_df[k][0]][n]),
                                                      complex_mult(cb[k][m2][ind_df[k][1]], h[k][ind_df[k][1]][n])),
                                                      complex_mult(cb[k][m3][ind_df[k][2]], h[k][ind_df[k][2]][n]));
                        complex_double_t d = complex_subst(y[k][n], sum);
                        f[k][m1][m2][m3] = -(1/N0)*(d.real*d.real + d.imag*d.imag);
                    }
                }
            }
        }

        double Ap           = log(1.0/M);
        double Igv[K][V][M] = {0};
        double Ivg[K][V][M] = {0};

        for (int k = 0; k < K; k++)
        {
            for (int v = 0; v < V; v++)
            {
                for (int m = 0; m < M; m++)
                {
                    Ivg[k][v][m] = Ap;
                }
            }
        }

        // Step 2: Iterative procedure

        for (int iter = 0; iter < Niter; iter++)
        {
            // Igv update

            for (int k = 0; k < K; k++)
            {

                for (int m1 = 0; m1 < M; m1++)
                {
                    double sIgv[M*M] __attribute__((aligned(64))) = {0};
                    for (int m2 = 0; m2 < M; m2++)
                    {
                        for (int m3 = 0; m3 < M; m3++)
                        {
                            sIgv[m2*M+m3] = f[k][m1][m2][m3] + Ivg[k][ind_df[k][1]][m2] + Ivg[k][ind_df[k][2]][m3];
                        }
                    }
                    Igv[k][ind_df[k][0]][m1] = log_sum_exp(sIgv, M*M);
                }

                for (int m2 = 0; m2 < M; m2++)
                {
                    double sIgv[M*M] __attribute__((aligned(64))) = {0};
                    for (int m1 = 0; m1 < M; m1++)
                    {
                        for(int m3 = 0; m3 < M; m3++)
                        {
                            sIgv[m1*M+m3] = f[k][m1][m2][m3]+ Ivg[k][ind_df[k][0]][m1] + Ivg[k][ind_df[k][2]][m3];
                        }
                    }
                    Igv[k][ind_df[k][1]][m2] = log_sum_exp(sIgv, M*M);
                }

                for (int m3 = 0; m3 < M; m3++)
                {
                    double sIgv[M*M] __attribute__((aligned(64))) = {0};
                    for (int m1 = 0; m1 < M; m1++)
                    {
                        for (int m2 = 0; m2 < M; m2++)
                        {
                            sIgv[m1*M+m2] = f[k][m1][m2][m3]+ Ivg[k][ind_df[k][0]][m1] + Ivg[k][ind_df[k][1]][m2];
                        }
                    }
                    Igv[k][ind_df[k][2]][m3] = log_sum_exp(sIgv, M*M);
                }
            }

            // Ivg update

            for (int v = 0; v < V; v++)
            {
                for (int m = 0; m < M; m++)
                {
                    double sum0 = 0;
                    double sum1 = 0;
                    for (int i = 0; i < M; i++)
                    {
                        sum0 += exp(Igv[ind_dv[v][0]][v][i]);
                        sum1 += exp(Igv[ind_dv[v][1]][v][i]);
                    }

                    Ivg[ind_dv[v][0]][v][m] = Igv[ind_dv[v][1]][v][m]-log(sum1);
                    Ivg[ind_dv[v][1]][v][m] = Igv[ind_dv[v][0]][v][m]-log(sum0);
                }
            }
        }

        // Step 3: LLR calculation

        double Q[M][V] = {0};

        for (int v = 0; v < V; v++)
        {
            for (int m = 0; m < M; m++)
            {
                Q[m][v] = Ap + Igv[ind_dv[v][0]][v][m] + Igv[ind_dv[v][1]][v][m];
            }
        }

        for (int v = 0; v < V; v++)
        {
            LLR[2*v][n]   = log((exp(Q[0][v])+exp(Q[1][v]))/((exp(Q[2][v])+exp(Q[3][v]))));
            LLR[2*v+1][n] = log((exp(Q[0][v])+exp(Q[2][v]))/((exp(Q[1][v])+exp(Q[3][v]))));
        }
    }
}
