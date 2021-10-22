#pragma once
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>

#include "fomalib.h"
#include "pyis/share/cached_object.h"
#include "pyis/share/exception.h"
#include "pyis/share/file_system.h"
#include "pyis/share/json_persist_helper.h"
#include "pyis/share/model_storage.h"
#include "pyis/share/model_storage_local.h"

namespace pyis {
namespace ops {

class FomaFst : public CachedObject<FomaFst> {
  public:
    explicit FomaFst(const std::string& bin_path);

    FomaFst() = default;

    ~FomaFst();

    std::string ApplyDown(const std::string& input) const;

    static void CompileFromFile(const std::string& infile);
    static void CompileFromStr(const std::string& infile_str);

    void SaveBinary(std::ostream& os);
    void LoadBinary(std::istream& os);

    void Reset();

    std::string Serialize(ModelStorage& storage);

    void Deserialize(const std::string& state, ModelStorage& storage);

  private:
    fsm* net_ = nullptr;
    apply_handle* handle_ = nullptr;

    bool initialize_fst(const std::string& bin_path);
    static void CompileInit();
};

}  // namespace ops
}  // namespace pyis
