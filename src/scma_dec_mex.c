/*
 * Saint Petersburg Electrotechnical University, Saint Petersburg, Russia
 * Faculty of Radio Engineering
 * Department of Theoretical Fundamentals of Radio Engineering
 * Vyacheslav P. Klimentyev, 2018
 */

#include "mex.h"
#include "scma_log_mpa.h"

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    (void)nlhs;
    (void)nrhs;

    mxComplexDouble *y_in;
    mxComplexDouble *cb_in;
    mxComplexDouble *h_in;
    mxDouble        *N0_in;
    mxInt32         *Niter_in;

    mxDouble        *LLR_out;

    y_in       = mxGetComplexDoubles(prhs[0]);
    cb_in      = mxGetComplexDoubles(prhs[1]);
    h_in       = mxGetComplexDoubles(prhs[2]);
    N0_in      = mxGetDoubles(prhs[3]);
    Niter_in   = mxGetInt32s(prhs[4]);

    plhs[0] = mxCreateDoubleMatrix(B, N, mxREAL);
    LLR_out = (mxDouble *)mxGetPr(plhs[0]);


    // Conversion of input args

    complex_double_t y[K][N]     = {0};
    complex_double_t cb[K][M][V] = {0};
    complex_double_t h[K][V][N]  = {0};
    double           N0;
    int              Niter;

    double LLR[B][N] = {0};

    for(int n = 0; n < N; n++)
    {
        for(int k = 0; k < K; k++)
        {
            y[k][n].real = y_in[n*K+k].real;
            y[k][n].imag = y_in[n*K+k].imag;
        }
    }

    for(int v = 0; v < V; v++)
    {
        for(int m = 0; m < M; m++)
        {
            for(int k = 0; k < K; k++)
            {
                cb[k][m][v].real = cb_in[v*M*K+m*K+k].real;
                cb[k][m][v].imag = cb_in[v*M*K+m*K+k].imag;
            }
        }
    }

    for(int n = 0; n < N; n++)
    {
        for(int v = 0; v < V; v++)
        {
            for(int k = 0; k < K; k++)
            {
                h[k][v][n].real = h_in[n*V*K+v*K+k].real;
                h[k][v][n].imag = h_in[n*V*K+v*K+k].imag;
            }
        }
    }


    N0    = (double)(*N0_in);
    Niter = (int)(*Niter_in);

    scmadec(y, cb, h, N0, Niter, LLR);

    //  Conversion of output arg

    for (int col = 0; col < N; col++)
    {
        for (int row = 0; row < B; row++)
        {
            LLR_out[row + col*B] = LLR[row][col];
        }
    }
}
