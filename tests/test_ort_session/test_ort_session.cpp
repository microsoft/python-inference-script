#include <codecvt>
#include <functional>
#include <iostream>
#include <locale>
#include <string>
#include <vector>

#include "gtest/gtest.h"
#include "pyis/ops/ort_session/ort_session.h"
#include "pyis/share/str_utils.h"

static std::string model_file = "tests/test_ort_session/data/enus_emotion.onnx";

TEST(TestOrtSession, DISABLED_DynamicBatching) {
    pyis::OrtGlobals::Initialize();
    std::string test_model_file = "tests/test_ort_session/data/uu.onnx";
    const std::vector<std::string> input_names{"input_ids", "attention_mask"};
    const std::vector<std::string> output_names{"outputs"};
    auto session = std::make_shared<pyis::ops::OrtSession>(test_model_file, input_names, output_names, 1, 0, true, 8);
    std::vector<std::shared_ptr<Ort::Value>> inputs;
    std::vector<int64_t> shape{1, 6};

    inputs.emplace_back(
        std::make_shared<Ort::Value>(Ort::Value::CreateTensor<int64_t>(*pyis::OrtGlobals::Allocator, shape.data(), 2)));
    inputs.emplace_back(
        std::make_shared<Ort::Value>(Ort::Value::CreateTensor<int64_t>(*pyis::OrtGlobals::Allocator, shape.data(), 2)));

    int64_t msg_id_data[6]{101, 4618, 2057, 2655, 1029, 102};
    memcpy(static_cast<void*>(inputs[0]->GetTensorMutableData<void>()), static_cast<void*>(msg_id_data),
           6 * sizeof(int64_t));

    int64_t msg_mask_data[6]{1, 1, 1, 1, 1, 1};
    memcpy(static_cast<void*>(inputs[1]->GetTensorMutableData<void>()), static_cast<void*>(msg_mask_data),
           6 * sizeof(int64_t));

    auto f = [&]() { return session->Run(inputs); };

    auto fut1 = std::async(std::launch::async, f);
    auto fut2 = std::async(std::launch::async, f);
    auto fut3 = std::async(std::launch::async, f);

    std::cout << fut1.get()[0]->GetTensorTypeAndShapeInfo().GetElementCount() << std::endl;
    std::cout << fut2.get()[0]->GetTensorTypeAndShapeInfo().GetElementCount() << std::endl;
    std::cout << fut3.get()[0]->GetTensorTypeAndShapeInfo().GetElementCount() << std::endl;
}

TEST(TestRunOrtSession, Basics) {
    pyis::OrtGlobals::Initialize();
    Ort::Env env(ORT_LOGGING_LEVEL_WARNING, "test_onnxruntime");
    Ort::SessionOptions session_options;
    session_options.SetIntraOpNumThreads(1);
    session_options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_EXTENDED);
    Ort::AllocatorWithDefaultOptions allocator;

    // [TODO_haoji] will fix the path later
#ifdef _WIN32
    Ort::Session session(env, pyis::str_to_wstr(model_file).c_str(), session_options);
#else
    Ort::Session session(env, model_file.c_str(), session_options);
#endif

    size_t num_input_nodes = session.GetInputCount();
    std::vector<const char*> input_node_names;
    input_node_names.reserve(num_input_nodes);
    for (auto i = 0; i < num_input_nodes; i++) {
        input_node_names.emplace_back(session.GetInputName(i, allocator));
    }

    size_t num_output_nodes = session.GetOutputCount();
    std::vector<const char*> output_node_names;
    output_node_names.reserve(num_output_nodes);
    for (auto i = 0; i < num_output_nodes; i++) {
        output_node_names.emplace_back(session.GetOutputName(i, allocator));
    }

    std::vector<int64_t> input_node_dims{10};
    std::vector<int64_t> input_1{1, 1, 1, 1, 1, 1, 1, 1, 1, 1};

    auto memory_info = Ort::MemoryInfo::CreateCpu(OrtAllocatorType::OrtArenaAllocator, OrtMemType::OrtMemTypeDefault);

    std::vector<Ort::Value> input_values;
    input_values.emplace_back(Ort::Value::CreateTensor<int64_t>(memory_info, input_1.data(), input_1.size(),
                                                                input_node_dims.data(), input_node_dims.size()));

    auto output = session.Run(Ort::RunOptions(), input_node_names.data(), input_values.data(), input_node_names.size(),
                              output_node_names.data(), output_node_names.size());

    ASSERT_TRUE(output[0].IsTensor());
}