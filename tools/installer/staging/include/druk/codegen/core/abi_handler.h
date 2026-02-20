#pragma once

#include <string>

namespace druk::codegen
{

/**
 * @brief Handles ABI-specific details like calling conventions and struct layout.
 */
class ABIHandler
{
   public:
    static std::string getCallingConvention();
};

}  // namespace druk::codegen
