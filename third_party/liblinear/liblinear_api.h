#pragma once

#include "linear.h"
#include <cstdio>
#include <cmath>

bool liblinear_train(const char* libsvm_data_file, 
                     const char* model_file, 
                     int solver_type = L2R_L2LOSS_SVC_DUAL, 
                     double eps = HUGE_VAL,
                     double c = 1, 
                     double p = 0.1, 
                     double bias = -1);

bool liblinear_save_model(model* model, FILE* f);

model* liblinear_load_model(FILE* f);