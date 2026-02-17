#ifdef DRUK_HAVE_LLVM

#include "druk/codegen/llvm/llvm_backend.h"

#include <llvm/ExecutionEngine/Orc/Core.h>

#include "druk/codegen/jit/jit_runtime.h"

extern "C"
{
    void druk_jit_index(const PackedValue* arr_val, const PackedValue* idx_val, PackedValue* out);
    void druk_jit_index_set(PackedValue* arr_val, const PackedValue* idx_val,
                            const PackedValue* val);
    void druk_jit_build_struct(const PackedValue* keys, const PackedValue* values, int32_t count,
                               PackedValue* out);
    void druk_jit_get_field(const PackedValue* struct_val, const char* field, size_t field_len,
                            PackedValue* out);
    void druk_jit_set_field(PackedValue* struct_val, const char* field, size_t field_len,
                            const PackedValue* val);
    void druk_jit_len(const PackedValue* val, PackedValue* out);
    void druk_jit_push(PackedValue* arr_val, const PackedValue* element);
    void druk_jit_pop_array(PackedValue* arr_val, PackedValue* out);
    void druk_jit_typeof(const PackedValue* val, PackedValue* out);
    void druk_jit_keys(const PackedValue* val, PackedValue* out);
    void druk_jit_values(const PackedValue* val, PackedValue* out);
    void druk_jit_contains(const PackedValue* container, const PackedValue* item, PackedValue* out);
    void druk_jit_input(PackedValue* out);
    void druk_jit_print(const PackedValue* val);
    void druk_jit_call(const PackedValue* callee, const PackedValue* args, int32_t arg_count,
                       PackedValue* out);
    void druk_jit_get_arg(int32_t index, PackedValue* out);
    void druk_jit_register_function(druk::codegen::ObjFunction* function,
                                    void (*fn)(PackedValue* out));
    void druk_jit_set_compile_handler(DrukJitCompileFn fn);
    int64_t druk_jit_value_as_int(const PackedValue* value);
    bool    druk_jit_value_as_bool(const PackedValue* value);
}

namespace druk::codegen
{

void LLVMBackend::register_extended_symbols()
{
    auto&                        jd = ctx_->jit->getMainJITDylib();
    llvm::orc::MangleAndInterner mangle(ctx_->jit->getExecutionSession(),
                                        ctx_->jit->getDataLayout());
    llvm::orc::SymbolMap         symbols;

    symbols[mangle("druk_jit_index")]     = {llvm::orc::ExecutorAddr::fromPtr(&druk_jit_index),
                                             llvm::JITSymbolFlags::Exported};
    symbols[mangle("druk_jit_index_set")] = {llvm::orc::ExecutorAddr::fromPtr(&druk_jit_index_set),
                                             llvm::JITSymbolFlags::Exported};
    symbols[mangle("druk_jit_build_struct")] = {
        llvm::orc::ExecutorAddr::fromPtr(&druk_jit_build_struct), llvm::JITSymbolFlags::Exported};
    symbols[mangle("druk_jit_get_field")] = {llvm::orc::ExecutorAddr::fromPtr(&druk_jit_get_field),
                                             llvm::JITSymbolFlags::Exported};
    symbols[mangle("druk_jit_set_field")] = {llvm::orc::ExecutorAddr::fromPtr(&druk_jit_set_field),
                                             llvm::JITSymbolFlags::Exported};
    symbols[mangle("druk_jit_len")]       = {llvm::orc::ExecutorAddr::fromPtr(&druk_jit_len),
                                             llvm::JITSymbolFlags::Exported};
    symbols[mangle("druk_jit_push")]      = {llvm::orc::ExecutorAddr::fromPtr(&druk_jit_push),
                                             llvm::JITSymbolFlags::Exported};
    symbols[mangle("druk_jit_pop_array")] = {llvm::orc::ExecutorAddr::fromPtr(&druk_jit_pop_array),
                                             llvm::JITSymbolFlags::Exported};
    symbols[mangle("druk_jit_typeof")]    = {llvm::orc::ExecutorAddr::fromPtr(&druk_jit_typeof),
                                             llvm::JITSymbolFlags::Exported};
    symbols[mangle("druk_jit_keys")]      = {llvm::orc::ExecutorAddr::fromPtr(&druk_jit_keys),
                                             llvm::JITSymbolFlags::Exported};
    symbols[mangle("druk_jit_values")]    = {llvm::orc::ExecutorAddr::fromPtr(&druk_jit_values),
                                             llvm::JITSymbolFlags::Exported};
    symbols[mangle("druk_jit_contains")]  = {llvm::orc::ExecutorAddr::fromPtr(&druk_jit_contains),
                                             llvm::JITSymbolFlags::Exported};
    symbols[mangle("druk_jit_input")]     = {llvm::orc::ExecutorAddr::fromPtr(&druk_jit_input),
                                             llvm::JITSymbolFlags::Exported};
    symbols[mangle("druk_jit_print")]     = {llvm::orc::ExecutorAddr::fromPtr(&druk_jit_print),
                                             llvm::JITSymbolFlags::Exported};
    symbols[mangle("druk_jit_call")]      = {llvm::orc::ExecutorAddr::fromPtr(&druk_jit_call),
                                             llvm::JITSymbolFlags::Exported};
    symbols[mangle("druk_jit_get_arg")]   = {llvm::orc::ExecutorAddr::fromPtr(&druk_jit_get_arg),
                                             llvm::JITSymbolFlags::Exported};
    symbols[mangle("druk_jit_register_function")] = {
        llvm::orc::ExecutorAddr::fromPtr(&druk_jit_register_function),
        llvm::JITSymbolFlags::Exported};
    symbols[mangle("druk_jit_set_compile_handler")] = {
        llvm::orc::ExecutorAddr::fromPtr(&druk_jit_set_compile_handler),
        llvm::JITSymbolFlags::Exported};
    symbols[mangle("druk_jit_value_as_int")] = {
        llvm::orc::ExecutorAddr::fromPtr(&druk_jit_value_as_int), llvm::JITSymbolFlags::Exported};
    symbols[mangle("druk_jit_value_as_bool")] = {
        llvm::orc::ExecutorAddr::fromPtr(&druk_jit_value_as_bool), llvm::JITSymbolFlags::Exported};

    llvm::cantFail(jd.define(llvm::orc::absoluteSymbols(std::move(symbols))));
}

}  // namespace druk::codegen

#endif  // DRUK_HAVE_LLVM
