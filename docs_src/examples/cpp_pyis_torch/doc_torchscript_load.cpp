#include "pyis/pyis_c_api.h"
#include <torch/script.h>

#include <iostream>

int main() {
    auto* pyis_api = GetPyisApi();
    auto* context = pyis_api->ModelContextCreate("[PATH_TO_PT_MODEL_FILE]", "");
    pyis_api->ModelContextActivate(context);

    torch::jit::script::Module module;
    try {
        module = torch::jit::load("[PATH_TO_PT_MODEL_FILE]");

        auto query = torch::IValue("what is the weather");
        auto locale = torch::IValue("en-us");
        
        auto output = module.forward({query, locale}).toBool();
        std::cout << "Weather intent : " << output << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "error loading or running the model " << e.what() << std::endl;
        return -1;
    }
}