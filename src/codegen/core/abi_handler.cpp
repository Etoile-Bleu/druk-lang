#include "druk/codegen/core/abi_handler.h"

#include <string>

namespace druk::codegen
{

/**
 * @brief Handles ABI-specific details like calling conventions and struct layout.
 */

std::string ABIHandler::getCallingConvention()
{
    return "ccc";  // C calling convention
}

}  // namespace druk::codegen
