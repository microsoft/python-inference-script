// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <cstring>
#include <unordered_set>

#include "Common.h"

namespace SparseLinearChainCRF {
// ------------------------------------------------
// Vector Computation Utils
// ------------------------------------------------
inline void CopyFloatToDoubleVector(const float* src, double* dst, size_t size) {
    for (int i = 0; i < size; ++i) {
        dst[i] = src[i];
    }
}

inline void CopyDoubleToFloatVector(const double* src, float* dst, size_t size) {
    for (int i = 0; i < size; ++i) {
        dst[i] = (float)src[i];
    }
}

inline void ExponentializeVector(double* vec, size_t size) {
    for (int i = 0; i < size; ++i) {
        vec[i] = exp(vec[i]);
    }
}

inline void ExponentializeVector(float* vec, size_t size) {
    for (int i = 0; i < size; ++i) {
        vec[i] = exp(vec[i]);
    }
}

inline static double VectorSum(const double* vec, size_t size) {
    double sum = 0.0;
    for (int i = 0; i < size; ++i) {
        sum += vec[i];
    }
    return sum;
}

inline static float VectorSum(const float* vec, size_t size) {
    float sum = 0.0f;
    for (int i = 0; i < size; ++i) {
        sum += vec[i];
    }
    return sum;
}

inline static void VectorScale(double* vec, double scale, size_t size) {
    for (int i = 0; i < size; ++i) {
        vec[i] *= scale;
    }
}

inline static void VectorScale(float* vec, float scale, size_t size) {
    for (int i = 0; i < size; ++i) {
        vec[i] *= scale;
    }
}

inline static void VectorAdd(double* lhs, const double* rhs, double val, size_t size) {
    for (int i = 0; i < size; ++i) {
        lhs[i] += rhs[i] * val;
    }
}

inline static void VectorAdd(float* lhs, const float* rhs, float val, size_t size) {
    for (int i = 0; i < size; ++i) {
        lhs[i] += rhs[i] * val;
    }
}

inline static void VectorSet(double* vec, double val, size_t size) {
    for (int i = 0; i < size; ++i) {
        vec[i] = val;
    }
}

inline static void VectorCopy(double* dst, const double* src, size_t size) { memcpy(dst, src, sizeof(double) * size); }

inline static void VectorCopy(float* dst, const float* src, size_t size) { memcpy(dst, src, sizeof(float) * size); }

inline static void VectorMultiply(double* lhs, const double* rhs, size_t size) {
    for (int i = 0; i < size; ++i) {
        lhs[i] *= rhs[i];
    }
}

inline static void VectorMultiply(float* lhs, const float* rhs, size_t size) {
    for (int i = 0; i < size; ++i) {
        lhs[i] *= rhs[i];
    }
}

inline static double VectorDotProduct(const double* vecA, const double* vecB, size_t size) {
    double sum = 0.0;
    for (int i = 0; i < size; ++i) {
        sum += vecA[i] * vecB[i];
    }
    return sum;
}

inline static double VectorSumLog(const double* vec, size_t size) {
    double sum = 0.0;
    for (int i = 0; i < size; ++i) {
        sum += log(vec[i]);
    }
    return sum;
}

inline static float VectorSumLog(const float* vec, size_t size) {
    float sum = 0.0f;
    for (int i = 0; i < size; ++i) {
        sum += log(vec[i]);
    }
    return sum;
}

inline static void SparseVectorAdd(double* lhs, const double* rhs, double val, const std::vector<uint16_t>& vec) {
    for (const auto& i : vec) {
        lhs[i] += (rhs[i] - 1.0) * val;
    }
}

inline static double SparseVectorDotProduct(const double* vecA, const double* vecB, const std::vector<uint16_t>& vec) {
    double sum = 0.0;
    for (const auto& i : vec) {
        sum += (vecA[i] - 1.0) * vecB[i];
    }
    return sum;
}

inline static void SparseVectorMultiply(double* lhs, const double* rhs, size_t size) {
    for (int i = 0; i < size; ++i) {
        lhs[i] = (lhs[i] + 1.0) * rhs[i];
    }
}

}  // namespace SparseLinearChainCRF