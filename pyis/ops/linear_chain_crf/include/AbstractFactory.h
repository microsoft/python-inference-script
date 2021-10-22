// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>

#include "src/Common.h"

namespace SparseLinearChainCRFConsole {
namespace {
template <typename ConcreteType, typename AbstractType>
std::shared_ptr<AbstractType> CreateInstance() {
    std::shared_ptr<AbstractType> ptr(new ConcreteType);
    return ptr;
}
}  // namespace

// ------------------------------------------------
// AbstractFactory
// ------------------------------------------------
template <typename AbstractType>
class AbstractFactory {
  public:
    virtual ~AbstractFactory() {}

    std::shared_ptr<AbstractType> Create(std::string className) const {
        const auto iter = m_FactoryMap.find(className);
        if (iter == m_FactoryMap.end()) {
            LogAssert(false, "Unknown Class Name %s", className.c_str());
        }
        return iter->second();
    }

    virtual void PrintHelp() const {
        for (const auto iter : m_FactoryHelp) {
            std::cout << iter.first << " : \t" << iter.second << std::endl;
        }
        std::cout << std::endl;
    }

  protected:
    void Register(std::function<std::shared_ptr<AbstractType>()> creator, std::string className, std::string desc) {
        m_FactoryMap[className] = creator;
        m_FactoryHelp.insert(std::make_pair(className, desc));
    }

    typedef std::function<std::shared_ptr<AbstractType>()> FactoryCreatorFnType;
    std::unordered_map<std::string, FactoryCreatorFnType> m_FactoryMap;
    std::unordered_map<std::string, std::string> m_FactoryHelp;
};
}  // namespace SparseLinearChainCRFConsole