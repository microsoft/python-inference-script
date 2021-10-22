#include "liblinear_api.h"
#include <clocale>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <ctype.h>
#include <cmath>
#include <cerrno>

// function from liblinear train.c(main)
#define Malloc(type, n) (type*)malloc((n) * sizeof(type))

void exit_input_error(int line_num) {
    fprintf(stderr, "Wrong input format at line %d\n", line_num);
    exit(1);
}

static char* readline(FILE* input, char* line, int max_line_len) {
    int len;

    if (fgets(line, max_line_len, input) == NULL) return NULL;

    while (strrchr(line, '\n') == NULL) {
        max_line_len *= 2;
        line = (char*)realloc(line, max_line_len);
        len = (int)strlen(line);
        if (fgets(line + len, max_line_len - len, input) == NULL) break;
    }
    return line;
}

// read in a problem (in libsvm format)
void read_problem(const char* filename, struct problem& prob, struct feature_node*& x_space, double& bias) {
    int max_index, inst_max_index, i;
    size_t elements, j;
    FILE* fp = fopen(filename, "r");
    char* endptr;
    char *idx, *val, *label;

    if (fp == NULL) {
        fprintf(stderr, "can't open input file %s\n", filename);
        exit(1);
    }

    prob.l = 0;
    elements = 0;
    const int max_line_len = 1024;
    char line[max_line_len];
    while (readline(fp, line, max_line_len) != NULL) {
        char* p = strtok(line, " \t");  // label

        // features
        while (1) {
            p = strtok(NULL, " \t");
            if (p == NULL || *p == '\n')  // check '\n' as ' ' may be after the last feature
                break;
            elements++;
        }
        elements++;  // for bias term
        prob.l++;
    }
    rewind(fp);

    prob.bias = bias;

    prob.y = Malloc(double, prob.l);
    prob.x = Malloc(struct feature_node*, prob.l);
    x_space = Malloc(struct feature_node, elements + prob.l);

    max_index = 0;
    j = 0;
    for (i = 0; i < prob.l; i++) {
        inst_max_index = 0;  // strtol gives 0 if wrong format
        readline(fp, line, max_line_len);
        prob.x[i] = &x_space[j];
        label = strtok(line, " \t\n");
        if (label == NULL)  // empty line
            exit_input_error(i + 1);

        prob.y[i] = strtod(label, &endptr);
        if (endptr == label || *endptr != '\0') exit_input_error(i + 1);

        while (1) {
            idx = strtok(NULL, ":");
            val = strtok(NULL, " \t");

            if (val == NULL) break;

            errno = 0;
            x_space[j].index = (int)strtol(idx, &endptr, 10);
            if (endptr == idx || errno != 0 || *endptr != '\0' || x_space[j].index <= inst_max_index)
                exit_input_error(i + 1);
            else
                inst_max_index = x_space[j].index;

            errno = 0;
            x_space[j].value = strtod(val, &endptr);
            if (endptr == val || errno != 0 || (*endptr != '\0' && !isspace(*endptr))) exit_input_error(i + 1);

            ++j;
        }

        if (inst_max_index > max_index) max_index = inst_max_index;

        if (prob.bias >= 0) x_space[j++].value = prob.bias;

        x_space[j++].index = -1;
    }

    if (prob.bias >= 0) {
        prob.n = max_index + 1;
        for (i = 1; i < prob.l; i++) (prob.x[i] - 2)->index = prob.n;
        x_space[j - 2].index = prob.n;
    } else
        prob.n = max_index;

    fclose(fp);
}

static const char* solver_type_table[] = {"L2R_LR",
                                          "L2R_L2LOSS_SVC_DUAL",
                                          "L2R_L2LOSS_SVC",
                                          "L2R_L1LOSS_SVC_DUAL",
                                          "MCSVM_CS",
                                          "L1R_L2LOSS_SVC",
                                          "L1R_LR",
                                          "L2R_LR_DUAL",
                                          "",
                                          "",
                                          "",
                                          "L2R_L2LOSS_SVR",
                                          "L2R_L2LOSS_SVR_DUAL",
                                          "L2R_L1LOSS_SVR_DUAL",
                                          "",
                                          "",
                                          "",
                                          "",
                                          "",
                                          "",
                                          "",
                                          "ONECLASS_SVM",
                                          NULL};

bool liblinear_train(const char* libsvm_data_file, const char* model_file, int solver_type, double eps, double c,
                     double p, double bias) {
    // fix the random seed to have consistent training results among different runs.
    srand(1);

    // default values
    parameter param;
    param.solver_type = solver_type;
    param.C = c;
    param.eps = eps;  // see setting below
    param.p = p;
    param.nr_weight = 0;
    param.weight_label = NULL;
    param.weight = NULL;
    param.init_sol = NULL;
    if (param.eps == HUGE_VAL) {
        switch (param.solver_type) {
            case L2R_LR:
            case L2R_L2LOSS_SVC:
                param.eps = 0.01;
                break;
            case L2R_L2LOSS_SVR:
                param.eps = 0.001;
                break;
            case L2R_L2LOSS_SVC_DUAL:
            case L2R_L1LOSS_SVC_DUAL:
            case MCSVM_CS:
            case L2R_LR_DUAL:
                param.eps = 0.1;
                break;
            case L1R_L2LOSS_SVC:
            case L1R_LR:
                param.eps = 0.01;
                break;
            case L2R_L1LOSS_SVR_DUAL:
            case L2R_L2LOSS_SVR_DUAL:
                param.eps = 0.1;
                break;
        }
    }

    // read problem.
    struct problem prob;
    struct feature_node* x_space;
    read_problem(libsvm_data_file, prob, x_space, bias);

    // check param
    const char* error_msg = check_parameter(&prob, &param);
    if (error_msg) {
        fprintf(stderr, "check parameter failed. %s", error_msg);
        return false;
    }

    model* model = train(&prob, &param);
    save_model(model_file, model);
    free(prob.y);
    free(prob.x);
    free(x_space);

    destroy_param(&model->param);
    free_and_destroy_model(&model);

    return true;
}

bool liblinear_save_model(model* model, FILE* f) {
    int i;
    int nr_feature = model->nr_feature;
    int n;
    const parameter& param = model->param;

    if (model->bias >= 0) {
        n = nr_feature + 1;
    } else {
        n = nr_feature;
    }
    int w_size = n;

    char* old_locale = setlocale(LC_ALL, NULL);
    if (old_locale) {
        old_locale = strdup(old_locale);
    }
    setlocale(LC_ALL, "C");

    int nr_w;
    if (model->nr_class == 2 && model->param.solver_type != MCSVM_CS) {
        nr_w = 1;
    } else {
        nr_w = model->nr_class;
    }

    fprintf(f, "solver_type %s\n", solver_type_table[param.solver_type]);
    fprintf(f, "nr_class %d\n", model->nr_class);

    if (model->label) {
        fprintf(f, "label");
        for (i = 0; i < model->nr_class; i++) {
            fprintf(f, " %d", model->label[i]);
        }
        fprintf(f, "\n");
    }

    fprintf(f, "nr_feature %d\n", nr_feature);

    fprintf(f, "bias %.17g\n", model->bias);

    if (check_oneclass_model(model)) {
        fprintf(f, "rho %.17g\n", model->rho);
    }

    fprintf(f, "w\n");
    for (i = 0; i < w_size; i++) {
        int j;
        for (j = 0; j < nr_w; j++) {
            fprintf(f, "%.17g ", model->w[i * nr_w + j]);
        }
        fprintf(f, "\n");
    }

    setlocale(LC_ALL, old_locale);
    free(old_locale);

    return true;
}

//
// FSCANF helps to handle fscanf failures.
// Its do-while block avoids the ambiguity when
// if (...)
//    FSCANF();
// is used
//
#define FSCANF(_stream, _format, _var)                          \
    do {                                                        \
        if (fscanf(_stream, _format, _var) != 1) {              \
            fprintf(stderr, "fscanf failed to read the model"); \
            return nullptr;                                     \
        }                                                       \
    } while (0)

model* liblinear_load_model(FILE* f) {
    int i;
    int nr_feature;
    int n;
    int nr_class;
    double bias;
    double rho;
    struct model* model = Malloc(struct model, 1);
    parameter& param = model->param;
    // parameters for training only won't be assigned, but arrays are assigned as NULL for safety
    param.nr_weight = 0;
    param.weight_label = NULL;
    param.weight = NULL;
    param.init_sol = NULL;

    model->label = NULL;

    char* old_locale = setlocale(LC_ALL, NULL);
    if (old_locale) {
        old_locale = strdup(old_locale);
    }
    setlocale(LC_ALL, "C");

    char cmd[81];
    while (1) {
        FSCANF(f, "%80s", cmd);
        if (strcmp(cmd, "solver_type") == 0) {
            FSCANF(f, "%80s", cmd);
            int i;
            for (i = 0; solver_type_table[i]; i++) {
                if (strcmp(solver_type_table[i], cmd) == 0) {
                    param.solver_type = i;
                    break;
                }
            }
            if (solver_type_table[i] == NULL) {
                fprintf(stderr, "unknown solver type.");
                return nullptr;
            }
        } else if (strcmp(cmd, "nr_class") == 0) {
            FSCANF(f, "%d", &nr_class);
            model->nr_class = nr_class;
        } else if (strcmp(cmd, "nr_feature") == 0) {
            FSCANF(f, "%d", &nr_feature);
            model->nr_feature = nr_feature;
        } else if (strcmp(cmd, "bias") == 0) {
            FSCANF(f, "%lf", &bias);
            model->bias = bias;
        } else if (strcmp(cmd, "rho") == 0) {
            FSCANF(f, "%lf", &rho);
            model->rho = rho;
        } else if (strcmp(cmd, "w") == 0) {
            break;
        } else if (strcmp(cmd, "label") == 0) {
            int nr_class = model->nr_class;
            model->label = Malloc(int, nr_class);
            for (int i = 0; i < nr_class; i++) {
                FSCANF(f, "%d", &model->label[i]);
            }
        } else {
            fprintf(stderr, "unknown text in model file: %s", cmd);
            return nullptr;
        }
    }

    nr_feature = model->nr_feature;
    if (model->bias >= 0) {
        n = nr_feature + 1;
    } else {
        n = nr_feature;
    }
    int w_size = n;
    int nr_w;
    if (nr_class == 2 && param.solver_type != MCSVM_CS) {
        nr_w = 1;
    } else {
        nr_w = nr_class;
    }

    model->w = Malloc(double, w_size* nr_w);
    for (i = 0; i < w_size; i++) {
        int j;
        for (j = 0; j < nr_w; j++) {
            FSCANF(f, "%lf ", &model->w[i * nr_w + j]);
        }
    }

    setlocale(LC_ALL, old_locale);
    free(old_locale);
    return model;
}