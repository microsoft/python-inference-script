#include "foma_fst.h"

#include <string>

#include "foma.h"

extern "C" {
extern void my_interfaceparse(char* my_string);
extern struct stack_entry* main_stack;
int promptmode;
int apply_direction;
}

namespace {
int stream_printf(std::ostream& out, const char* format, ...) {
    va_list arglist;
    va_start(arglist, format);
    char buffer[1024];
    int sz = vsnprintf(buffer, sizeof(buffer), format, arglist);
    out.write(buffer, sz);
    va_end(arglist);
    return sz;
}

void foma_net_print(struct fsm* net, std::ostream& out) {
    struct sigma* sigma;
    struct fsm_state* fsm;
    int i;
    int maxsigma;
    int laststate;
    int* cm;
    int extras;

    /* Header */
    stream_printf(out, "%s", "##foma-net 1.0##\n");

    /* Properties */
    stream_printf(out, "%s", "##props##\n");

    extras = (net->is_completed) | (net->arcs_sorted_in << 2) | (net->arcs_sorted_out << 4);

    stream_printf(out, "%i %i %i %i %i %lld %i %i %i %i %i %i %s\n", net->arity, net->arccount, net->statecount,
                  net->linecount, net->finalcount, net->pathcount, net->is_deterministic, net->is_pruned,
                  net->is_minimized, net->is_epsilon_free, net->is_loop_free, extras, net->name);

    /* Sigma */
    stream_printf(out, "%s", "##sigma##\n");
    for (sigma = net->sigma; sigma != nullptr && sigma->number != -1; sigma = sigma->next) {
        stream_printf(out, "%i %s\n", sigma->number, sigma->symbol);
    }

    /* State array */
    laststate = -1;
    stream_printf(out, "%s", "##states##\n");
    for (fsm = net->states; fsm->state_no != -1; fsm++) {
        if (fsm->state_no != laststate) {
            if (fsm->in != fsm->out) {
                stream_printf(out, "%i %i %i %i %i\n", fsm->state_no, fsm->in, fsm->out, fsm->target, fsm->final_state);
            } else {
                stream_printf(out, "%i %i %i %i\n", fsm->state_no, fsm->in, fsm->target, fsm->final_state);
            }
        } else {
            if (fsm->in != fsm->out) {
                stream_printf(out, "%i %i %i\n", fsm->in, fsm->out, fsm->target);
            } else {
                stream_printf(out, "%i %i\n", fsm->in, fsm->target);
            }
        }
        laststate = fsm->state_no;
    }
    /* Sentinel for states */
    stream_printf(out, "-1 -1 -1 -1 -1\n");

    /* Store confusion matrix */
    if (net->medlookup != nullptr && net->medlookup->confusion_matrix != nullptr) {
        stream_printf(out, "%s", "##cmatrix##\n");
        cm = net->medlookup->confusion_matrix;
        maxsigma = sigma_max(net->sigma) + 1;
        for (i = 0; i < maxsigma * maxsigma; i++) {
            stream_printf(out, "%i\n", *(cm + i));
        }
    }

    /* End */
    stream_printf(out, "%s", "##end##\n");
}

}  // namespace

namespace pyis {
namespace ops {

using std::string;

FomaFst::FomaFst(const string& bin_path) { this->initialize_fst(bin_path); }

FomaFst::~FomaFst() { Reset(); }

void FomaFst::Reset() {
    if (handle_ != nullptr) {
        apply_clear(handle_);
        handle_ = nullptr;
    }
    if (net_ != nullptr) {
        fsm_destroy(net_);
        net_ = nullptr;
    }
}

void FomaFst::SaveBinary(std::ostream& os) {
    if (net_ == nullptr) {
        PYIS_THROW("FomaFst is empty, save fail");
    }
    foma_net_print(net_, os);
}

void FomaFst::LoadBinary(std::istream& os) {
    Reset();
    os.seekg(0, std::istream::end);
    size_t length = os.tellg();
    os.seekg(0, std::istream::beg);
    if (length == 0) {
        PYIS_THROW("Load FomaFst binary fail");
    }

    struct io_buf_handle* iobh;
    char* net_name;
    iobh = io_init();
    iobh->io_buf = reinterpret_cast<char*>(malloc(length + 1));
    *((iobh->io_buf) + length) = '\0';
    iobh->io_buf_ptr = iobh->io_buf;
    os.read(iobh->io_buf, length);

    net_ = io_net_read(iobh, &net_name);
    free(net_name);
    io_free(iobh);
    handle_ = apply_init(net_);
    if (net_ == nullptr) {
        PYIS_THROW("Load FomaFst binary fail");
    }
}

string FomaFst::Serialize(ModelStorage& storage) {
    string fst_bin = storage.uniq_file("foma_fst", ".fst");
    SaveBinary(*storage.open_ostream(fst_bin));

    JsonPersistHelper jph(1);
    jph.add_file("fst_bin_file", fst_bin);
    string state = jph.serialize();
    return state;
}

void FomaFst::Deserialize(const string& state, ModelStorage& storage) {
    JsonPersistHelper jph(state, storage);
    int version = jph.version();
    if (1 == version) {
        string fst_bin = jph.get_file("fst_bin_file");
        LoadBinary(*storage.open_istream(fst_bin));
    } else {
        PYIS_THROW("FomaFst v%d is incompatible with the runtime", version);
    }
}

bool FomaFst::initialize_fst(const string& bin_path) {
    Reset();
    char* bin_path_cstr = strdup(bin_path.c_str());
    net_ = fsm_read_binary_file(bin_path_cstr);
    free(bin_path_cstr);
    if (net_ == nullptr) {
        PYIS_THROW("FomaFst initialize error due to incorrect fst fomat");
        return false;
    }
    handle_ = apply_init(net_);
    return true;
}

string FomaFst::ApplyDown(const std::string& input) const {
    char* input_cstr = strdup(input.c_str());
    char* result_cstr = apply_down(handle_, input_cstr);
    free(input_cstr);
    if (result_cstr != nullptr) {
        return string(result_cstr);
    }
    return "";
}

void FomaFst::CompileInit() {
    promptmode = PROMPT_MAIN;
    if (main_stack != nullptr) {
        stack_clear();
    }
    stack_init();
    if (g_defines == nullptr) {
        g_defines = defined_networks_init();
    }
    if (g_defines_f == nullptr) {
        g_defines_f = defined_functions_init();
    }
    remove_defined(g_defines, nullptr);
}

void FomaFst::CompileFromFile(const std::string& infile) {
    char* infile_cstr = strdup(infile.c_str());
    char* buffer = file_to_mem(infile_cstr);
    free(infile_cstr);
    CompileInit();
    my_interfaceparse(buffer);
    free(buffer);
}

void FomaFst::CompileFromStr(const std::string& infile_str) {
    CompileInit();
    char* infile_str_cstr = strdup(infile_str.c_str());
    my_interfaceparse(infile_str_cstr);
    free(infile_str_cstr);
}

}  // namespace ops
}  // namespace pyis
