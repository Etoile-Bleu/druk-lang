#pragma once

#include <memory>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "druk/codegen/core/chunk.h"
#include "druk/codegen/core/obj.h"
#include "druk/codegen/core/value.h"

namespace druk::gc
{
class GcString;
}

namespace druk
{

enum class InterpretResult
{
    Ok,
    CompileError,
    RuntimeError
};

namespace vm
{

struct CallFrame
{
    codegen::ObjFunction* function;
    const uint8_t*        ip;
    codegen::Value*       slots;
};

class VM
{
   public:
    VM();
    ~VM();

    InterpretResult       interpret(codegen::ObjFunction* function);
    void                  set_args(const std::vector<std::string>& args);
    const codegen::Value& lastResult() const
    {
        return lastResult_;
    }

   private:
    InterpretResult run();

    void                  push(codegen::Value value);
    codegen::Value        pop();
    const codegen::Value& peek(int distance) const;

    void          runtimeError(const char* format, ...);
    gc::GcString* storeString(std::string value);

    size_t stackSize() const
    {
        return static_cast<size_t>(stackTop_ - stackBase_);
    }

    std::vector<CallFrame> frames_;
    CallFrame*             frame_ = nullptr;

    static constexpr size_t     kStackMax = 256 * 64;
    std::vector<codegen::Value> stack_;
    codegen::Value*             stackBase_ = nullptr;
    codegen::Value*             stackTop_  = nullptr;

    std::unordered_map<std::string_view, codegen::Value> globals_;
    size_t                                               globalsVersion_ = 0;
    struct GlobalCache
    {
        std::string_view name{};
        codegen::Value*  slot    = nullptr;
        size_t           version = 0;
    } globalCache_;

    std::vector<std::string> argvStorage_;

    codegen::Value lastResult_{};

    friend class VMTest;
};

}  // namespace vm
}  // namespace druk
