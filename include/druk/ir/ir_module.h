#pragma once

#include <map>
#include <memory>
#include <string>

#include "druk/ir/ir_function.h"


namespace druk::ir
{

/**
 * @brief Top-level container for IR functions and global variables.
 */
class Module
{
   public:
    explicit Module(const std::string& name);

    const std::string& getName() const
    {
        return name_;
    }

    void      addFunction(std::unique_ptr<Function> func);
    Function* getFunction(const std::string& name) const;

    const std::map<std::string, std::unique_ptr<Function>>& getFunctions() const
    {
        return functions_;
    }

   private:
    std::string                                      name_;
    std::map<std::string, std::unique_ptr<Function>> functions_;
};

}  // namespace druk::ir
