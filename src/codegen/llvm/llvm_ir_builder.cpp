#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

namespace druk::codegen
{

/**
 * @brief Wrapper for LLVM's IRBuilder to maintain project consistency.
 */
class LLVMIRBuilder
{
   public:
    explicit LLVMIRBuilder(llvm::LLVMContext& context) : builder_(context) {}

    llvm::IRBuilder<>& getBuilder()
    {
        return builder_;
    }

   private:
    llvm::IRBuilder<> builder_;
};

}  // namespace druk::codegen
