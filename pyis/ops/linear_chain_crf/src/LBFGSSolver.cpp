// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

// lbfgs.c was ported from the FORTRAN code of lbfgs.m to C
// using f2c converter (by Taku Kudo)
// http://www.ece.northwestern.edu/~nocedal/lbfgs.html
//
// Software for Large-scale Unconstrained Optimization
// L-BFGS is a limited-memory quasi-Newton code for unconstrained optimization.
// The code has been developed at the Optimization Technology Center,
// a joint venture of Argonne National Laboratory and Northwestern University.
//
// Authors
// Jorge Nocedal
//
// References
// (1) J. Nocedal. Updating Quasi-Newton Matrices with Limited Storage(1980),
// Mathematics of Computation 35, pp. 773-782.
// (2) D.C. Liu and J. Nocedal. On the Limited Memory Method for Large Scale Optimization(1989),
// Mathematical Programming B, 45, 3, pp. 503-528.
//
// Note: Modified minimally even though a generated code has bad coding convention.

#include "LBFGSSolver.h"

#include <cmath>
#include <iostream>
#include <numeric>

namespace SparseLinearChainCRF {
#define min(a, b) ((a) <= (b) ? (a) : (b))
#define max(a, b) ((a) >= (b) ? (a) : (b))

namespace {
static const float ftol = 1e-4f;
static const float xtol = 1e-16f;
static const float eps = 1e-7f;
static const float lb3_1_gtol = 0.9f;
static const float lb3_1_stpmin = 1e-20f;
static const float lb3_1_stpmax = 1e20f;
static const int lb3_1_mp = 6;
static const int lb3_1_lp = 6;

inline float pi(float x, float y) { return sigma(x) == sigma(y) ? x : 0.0f; }

inline void daxpy_(int n, float da, const float* dx, float* dy) {
    for (int i = 0; i < n; ++i) dy[i] += da * dx[i];
}

inline float ddot_(int size, const float* dx, const float* dy) { return std::inner_product(dx, dx + size, dy, 0.0f); }

void mcstep(float* stx, float* fx, float* dx, float* sty, float* fy, float* dy, float* stp, float fp, float dp,
            int* brackt, float stpmin, float stpmax, int* info) {
    bool bound = true;
    float p, q, s, d1, d2, d3, r, gamma, theta, stpq, stpc, stpf;
    *info = 0;

    if (*brackt &&
        ((*stp <= min(*stx, *sty) || *stp >= max(*stx, *sty)) || *dx * (*stp - *stx) >= 0.0 || stpmax < stpmin)) {
        return;
    }

    float sgnd = dp * (*dx / std::abs(*dx));

    if (fp > *fx) {
        *info = 1;
        bound = true;
        theta = (*fx - fp) * 3 / (*stp - *stx) + *dx + dp;
        d1 = std::abs(theta);
        d2 = std::abs(*dx);
        d1 = max(d1, d2);
        d2 = std::abs(dp);
        s = max(d1, d2);
        d1 = theta / s;
        gamma = s * std::sqrt(d1 * d1 - *dx / s * (dp / s));
        if (*stp < *stx) {
            gamma = -gamma;
        }
        p = gamma - *dx + theta;
        q = gamma - *dx + gamma + dp;
        r = p / q;
        stpc = *stx + r * (*stp - *stx);
        stpq = *stx + *dx / ((*fx - fp) / (*stp - *stx) + *dx) / 2 * (*stp - *stx);
        if ((d1 = stpc - *stx, std::abs(d1)) < (d2 = stpq - *stx, std::abs(d2))) {
            stpf = stpc;
        } else {
            stpf = stpc + (stpq - stpc) / 2;
        }
        *brackt = true;
    } else if (sgnd < 0.0) {
        *info = 2;
        bound = false;
        theta = (*fx - fp) * 3 / (*stp - *stx) + *dx + dp;
        d1 = std::abs(theta);
        d2 = std::abs(*dx);
        d1 = max(d1, d2);
        d2 = std::abs(dp);
        s = max(d1, d2);
        d1 = theta / s;
        gamma = s * std::sqrt(d1 * d1 - *dx / s * (dp / s));
        if (*stp > *stx) {
            gamma = -gamma;
        }
        p = gamma - dp + theta;
        q = gamma - dp + gamma + *dx;
        r = p / q;
        stpc = *stp + r * (*stx - *stp);
        stpq = *stp + dp / (dp - *dx) * (*stx - *stp);
        if ((d1 = stpc - *stp, std::abs(d1)) > (d2 = stpq - *stp, std::abs(d2))) {
            stpf = stpc;
        } else {
            stpf = stpq;
        }
        *brackt = true;
    } else if (std::abs(dp) < std::abs(*dx)) {
        *info = 3;
        bound = true;
        theta = (*fx - fp) * 3 / (*stp - *stx) + *dx + dp;
        d1 = std::abs(theta);
        d2 = std::abs(*dx);
        d1 = max(d1, d2);
        d2 = std::abs(dp);
        s = max(d1, d2);
        d3 = theta / s;
        d1 = 0.0;
        d2 = d3 * d3 - *dx / s * (dp / s);
        gamma = s * std::sqrt((max(d1, d2)));
        if (*stp > *stx) {
            gamma = -gamma;
        }
        p = gamma - dp + theta;
        q = gamma + (*dx - dp) + gamma;
        r = p / q;
        if (r < 0.0 && gamma != 0.0) {
            stpc = *stp + r * (*stx - *stp);
        } else if (*stp > *stx) {
            stpc = stpmax;
        } else {
            stpc = stpmin;
        }
        stpq = *stp + dp / (dp - *dx) * (*stx - *stp);
        if (*brackt) {
            if ((d1 = *stp - stpc, std::abs(d1)) < (d2 = *stp - stpq, std::abs(d2))) {
                stpf = stpc;
            } else {
                stpf = stpq;
            }
        } else {
            if ((d1 = *stp - stpc, std::abs(d1)) > (d2 = *stp - stpq, std::abs(d2))) {
                stpf = stpc;
            } else {
                stpf = stpq;
            }
        }
    } else {
        *info = 4;
        bound = false;
        if (*brackt) {
            theta = (fp - *fy) * 3 / (*sty - *stp) + *dy + dp;
            d1 = std::abs(theta);
            d2 = std::abs(*dy);
            d1 = max(d1, d2);
            d2 = std::abs(dp);
            s = max(d1, d2);
            d1 = theta / s;
            gamma = s * std::sqrt(d1 * d1 - *dy / s * (dp / s));
            if (*stp > *sty) {
                gamma = -gamma;
            }
            p = gamma - dp + theta;
            q = gamma - dp + gamma + *dy;
            r = p / q;
            stpc = *stp + r * (*sty - *stp);
            stpf = stpc;
        } else if (*stp > *stx) {
            stpf = stpmax;
        } else {
            stpf = stpmin;
        }
    }

    if (fp > *fx) {
        *sty = *stp;
        *fy = fp;
        *dy = dp;
    } else {
        if (sgnd < 0.0) {
            *sty = *stx;
            *fy = *fx;
            *dy = *dx;
        }
        *stx = *stp;
        *fx = fp;
        *dx = dp;
    }

    stpf = min(stpmax, stpf);
    stpf = max(stpmin, stpf);
    *stp = stpf;
    if (*brackt && bound) {
        if (*sty > *stx) {
            d1 = *stx + (*sty - *stx) * 0.66f;
            *stp = min(d1, *stp);
        } else {
            d1 = *stx + (*sty - *stx) * 0.66f;
            *stp = max(d1, *stp);
        }
    }

    return;
}
}  // namespace

class LBFGS::Mcsrch {
  private:
    int infoc, stage1, brackt;
    float finit, dginit, dgtest, width, width1;
    float stx, fx, dgx, sty, fy, dgy, stmin, stmax;

  public:
    Mcsrch()
        : infoc(0),
          stage1(0),
          brackt(0),
          finit(0.0f),
          dginit(0.0f),
          dgtest(0.0f),
          width(0.0f),
          width1(0.0f),
          stx(0.0f),
          fx(0.0f),
          dgx(0.0f),
          sty(0.0f),
          fy(0.0f),
          dgy(0.0f),
          stmin(0.0f),
          stmax(0.0f) {}

    void mcsrch(int size, float* x, float f, const float* g, float* s, float* stp, int* info, int* nfev, float* wa,
                bool orthant, float C) {
        static const float p5 = 0.5f;
        static const float p66 = 0.66f;
        static const float xtrapf = 4.0f;
        static const int maxfev_ = 20;

        /* Parameter adjustments */
        --wa;
        --s;
        --g;
        --x;

        if (*info == -1) goto L45;
        infoc = 1;

        if (size <= 0 || *stp <= 0.0) return;

        dginit = ddot_(size, &g[1], &s[1]);
        if (dginit >= 0.0) return;

        brackt = false;
        stage1 = true;
        *nfev = 0;
        finit = f;
        dgtest = ftol * dginit;
        width = lb3_1_stpmax - lb3_1_stpmin;
        width1 = width / p5;
        for (int j = 1; j <= size; ++j) {
            wa[j] = x[j];
        }

        stx = 0.0;
        fx = finit;
        dgx = dginit;
        sty = 0.0;
        fy = finit;
        dgy = dginit;

        while (true) {
            if (brackt) {
                stmin = min(stx, sty);
                stmax = max(stx, sty);
            } else {
                stmin = stx;
                stmax = *stp + xtrapf * (*stp - stx);
            }

            *stp = max(*stp, lb3_1_stpmin);
            *stp = min(*stp, lb3_1_stpmax);

            if ((brackt && ((*stp <= stmin || *stp >= stmax) || *nfev >= maxfev_ - 1 || infoc == 0)) ||
                (brackt && (stmax - stmin <= xtol * stmax))) {
                *stp = stx;
            }

            if (orthant) {
                for (int j = 1; j <= size; ++j) {
                    float grad_neg = 0.0f;
                    float grad_pos = 0.0f;
                    float grad = 0.0f;
                    if (wa[j] == 0.0f) {
                        grad_neg = g[j] - 1.0f / C;
                        grad_pos = g[j] + 1.0f / C;
                    } else {
                        grad_pos = grad_neg = g[j] + 1.0f * sigma(wa[j]) / C;
                    }
                    if (grad_neg > 0.0) {
                        grad = grad_neg;
                    } else if (grad_pos < 0.0) {
                        grad = grad_pos;
                    } else {
                        grad = 0.0;
                    }
                    const float p = pi(s[j], -grad);
                    const float xi = wa[j] == 0.0 ? sigma(-grad) : sigma(wa[j]);
                    x[j] = pi(wa[j] + *stp * p, xi);
                }
            } else {
                for (int j = 1; j <= size; ++j) {
                    x[j] = wa[j] + *stp * s[j];
                }
            }
            *info = -1;
            return;

        L45:
            *info = 0;
            ++(*nfev);
            float dg = ddot_(size, &g[1], &s[1]);
            float ftest1 = finit + *stp * dgtest;

            if (brackt && ((*stp <= stmin || *stp >= stmax) || infoc == 0)) {
                *info = 6;
            }
            if (*stp == lb3_1_stpmax && f <= ftest1 && dg <= dgtest) {
                *info = 5;
            }
            if (*stp == lb3_1_stpmin && (f > ftest1 || dg >= dgtest)) {
                *info = 4;
            }
            if (*nfev >= maxfev_) {
                *info = 3;
            }
            if (brackt && stmax - stmin <= xtol * stmax) {
                *info = 2;
            }
            if (f <= ftest1 && std::abs(dg) <= lb3_1_gtol * (-dginit)) {
                *info = 1;
            }

            if (*info != 0) {
                return;
            }

            if (stage1 && f <= ftest1 && dg >= min(ftol, lb3_1_gtol) * dginit) {
                stage1 = false;
            }

            if (stage1 && f <= fx && f > ftest1) {
                float fm = f - *stp * dgtest;
                float fxm = fx - stx * dgtest;
                float fym = fy - sty * dgtest;
                float dgm = dg - dgtest;
                float dgxm = dgx - dgtest;
                float dgym = dgy - dgtest;
                mcstep(&stx, &fxm, &dgxm, &sty, &fym, &dgym, stp, fm, dgm, &brackt, stmin, stmax, &infoc);
                fx = fxm + stx * dgtest;
                fy = fym + sty * dgtest;
                dgx = dgxm + dgtest;
                dgy = dgym + dgtest;
            } else {
                mcstep(&stx, &fx, &dgx, &sty, &fy, &dgy, stp, f, dg, &brackt, stmin, stmax, &infoc);
            }

            if (brackt) {
                float d1 = 0.0;
                if ((d1 = sty - stx, std::abs(d1)) >= p66 * width1) {
                    *stp = stx + p5 * (sty - stx);
                }
                width1 = width;
                width = (d1 = sty - stx, std::abs(d1));
            }
        }

        return;
    }
};

void LBFGS::clear() {
    iflag_ = iscn = nfev = iycn = point = npt = iter = info = ispt = isyt = iypt = 0;
    stp = stp1 = 0.0f;
    diag_.clear();
    w_.clear();
    delete mcsrch_;
    mcsrch_ = 0;
}

void LBFGS::lbfgs_optimize(int size, int msize, float* x, float f, const float* g, float* diag, float* w, bool orthant,
                           float C, int* iflag) {
    float yy = 0.0f;
    float ys = 0.0f;
    int bound = 0;
    int cp = 0;

    --diag;
    --g;
    --x;
    --w;

    if (!mcsrch_) mcsrch_ = new Mcsrch;

    if (*iflag == 1) goto L172;
    if (*iflag == 2) goto L100;

    // initialization
    if (*iflag == 0) {
        point = 0;
        for (int i = 1; i <= size; ++i) {
            diag[i] = 1.0;
        }
        ispt = size + (msize << 1);
        iypt = ispt + size * msize;
        for (int i = 1; i <= size; ++i) {
            w[ispt + i] = -g[i] * diag[i];
        }
        stp1 = 1.0f / std::sqrt(ddot_(size, &g[1], &g[1]));
    }

    // MAIN ITERATION LOOP
    while (true) {
        ++iter;
        info = 0;
        if (iter == 1) goto L165;
        if (iter > size) bound = size;

        // COMPUTE -H*G USING THE FORMULA GIVEN IN: Nocedal, J. 1980,
        // "Updating quasi-Newton matrices with limited storage",
        // Mathematics of Computation, Vol.24, No.151, pp. 773-782.
        ys = ddot_(size, &w[iypt + npt + 1], &w[ispt + npt + 1]);
        yy = ddot_(size, &w[iypt + npt + 1], &w[iypt + npt + 1]);
        for (int i = 1; i <= size; ++i) {
            diag[i] = ys / yy;
        }

    L100:
        cp = point;
        if (point == 0) cp = msize;
        w[size + cp] = 1.0f / ys;

        for (int i = 1; i <= size; ++i) {
            w[i] = -g[i];
        }

        bound = min(iter - 1, msize);

        cp = point;
        for (int i = 1; i <= bound; ++i) {
            --cp;
            if (cp == -1) cp = msize - 1;
            float sq = ddot_(size, &w[ispt + cp * size + 1], &w[1]);
            int inmc = size + msize + cp + 1;
            iycn = iypt + cp * size;
            w[inmc] = w[size + cp + 1] * sq;
            float d = -w[inmc];
            daxpy_(size, d, &w[iycn + 1], &w[1]);
        }

        for (int i = 1; i <= size; ++i) {
            w[i] = diag[i] * w[i];
        }

        for (int i = 1; i <= bound; ++i) {
            float yr = ddot_(size, &w[iypt + cp * size + 1], &w[1]);
            float beta = w[size + cp + 1] * yr;
            int inmc = size + msize + cp + 1;
            beta = w[inmc] - beta;
            iscn = ispt + cp * size;
            daxpy_(size, beta, &w[iscn + 1], &w[1]);
            ++cp;
            if (cp == msize) cp = 0;
        }

        // STORE THE NEW SEARCH DIRECTION
        for (int i = 1; i <= size; ++i) {
            w[ispt + point * size + i] = w[i];
        }

    L165:
        // OBTAIN THE ONE-DIMENSIONAL MINIMIZER OF THE FUNCTION
        // BY USING THE LINE SEARCH ROUTINE MCSRCH
        nfev = 0;
        stp = 1.0;
        if (iter == 1) {
            stp = stp1;
        }
        for (int i = 1; i <= size; ++i) {
            w[i] = g[i];
        }

    L172:
        mcsrch_->mcsrch(size, &x[1], f, &g[1], &w[ispt + point * size + 1], &stp, &info, &nfev, &diag[1], orthant, C);
        if (info == -1) {
            *iflag = 1;  // next value
            return;
        }
        if (info != 1) {
            std::cerr << "The line search routine mcsrch failed: error code:" << info << std::endl;
            *iflag = -1;
            return;
        }

        // COMPUTE THE NEW STEP AND GRADIENT CHANGE
        npt = point * size;
        for (int i = 1; i <= size; ++i) {
            w[ispt + npt + i] = stp * w[ispt + npt + i];
            w[iypt + npt + i] = g[i] - w[i];
        }
        ++point;
        if (point == msize) point = 0;

        float gnorm = std::sqrt(ddot_(size, &g[1], &g[1]));
        float xnorm = max(1.0f, std::sqrt(ddot_(size, &x[1], &x[1])));
        if (gnorm / xnorm <= eps) {
            *iflag = 0;  // OK terminated
            return;
        }
    }

    return;
}
}  // namespace SparseLinearChainCRF