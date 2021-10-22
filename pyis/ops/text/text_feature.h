// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <string>
#include <tuple>

namespace pyis {
namespace ops {

class TextFeature {
  public:
    TextFeature() : id_(-1), value_(0.0), start_(0), end_(-1) {}
    TextFeature(uint64_t id, double weight) : TextFeature() {
        id_ = id;
        value_ = weight;
    }
    TextFeature(uint64_t id, double weight, uint32_t start, uint32_t end) {
        id_ = id;
        value_ = weight;
        start_ = start;
        end_ = end;
    }
    virtual ~TextFeature() = default;
    TextFeature(const TextFeature& o) = default;
    TextFeature(TextFeature&& o) = default;

    uint64_t id() const { return id_; }
    void set_id(uint64_t id) { id_ = id; }

    double value() const { return value_; }
    void set_value(double value) { value_ = value; }

    std::string str() const { return str_; }
    void set_str(const std::string& str) { str_ = str; }

    int start() const { return start_; }
    void start(int start) { start_ = start; }

    int end() const { return end_; }
    void end(int end) { end_ = end; }

    std::tuple<int, int> pos() const {
        std::tuple<int, int> res(start_, end_);
        return res;
    }

    std::tuple<uint64_t, double, int, int> to_tuple() const {
        std::tuple<uint64_t, double, int, int> res(id_, value_, start_, end_);
        return res;
    }

  private:
    uint64_t id_;
    std::string str_;
    double value_;
    int start_;
    int end_;
};

}  // namespace ops
}  // namespace pyis