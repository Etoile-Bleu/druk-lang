#ifdef DRUK_HAVE_LLVM

#include "druk/codegen/llvm/llvm_backend.h"

#include <llvm/ExecutionEngine/Orc/Core.h>

#include "druk/codegen/jit/jit_runtime.h"

extern "C"
{
    void druk_jit_value_nil(PackedValue* out);
    void druk_jit_value_bool(bool b, PackedValue* out);
    void druk_jit_value_int(int64_t i, PackedValue* out);
    void druk_jit_value_function(druk::codegen::ObjFunction* fn, PackedValue* out);
    void druk_jit_add(const PackedValue* a, const PackedValue* b, PackedValue* out);
    void druk_jit_subtract(const PackedValue* a, const PackedValue* b, PackedValue* out);
    void druk_jit_multiply(const PackedValue* a, const PackedValue* b, PackedValue* out);
    void druk_jit_divide(const PackedValue* a, const PackedValue* b, PackedValue* out);
    void druk_jit_negate(const PackedValue* a, PackedValue* out);
    void druk_jit_equal(const PackedValue* a, const PackedValue* b, PackedValue* out);
    void druk_jit_less(const PackedValue* a, const PackedValue* b, PackedValue* out);
    void druk_jit_less_equal(const PackedValue* a, const PackedValue* b, PackedValue* out);
    void druk_jit_greater(const PackedValue* a, const PackedValue* b, PackedValue* out);
    void druk_jit_greater_equal(const PackedValue* a, const PackedValue* b, PackedValue* out);
    void druk_jit_not(const PackedValue* a, PackedValue* out);
    void druk_jit_get_global(const char* name, size_t name_len, PackedValue* out);
    void druk_jit_define_global(const char* name, size_t name_len, const PackedValue* val);
    void druk_jit_set_global(const char* name, size_t name_len, const PackedValue* val);
    void druk_jit_build_array(const PackedValue* elements, int32_t count, PackedValue* out);
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

void LLVMBackend::register_runtime_symbols()
{
    auto&                        jd = ctx_->jit->getMainJITDylib();
    llvm::orc::MangleAndInterner mangle(ctx_->jit->getExecutionSession(),
                                        ctx_->jit->getDataLayout());
    llvm::orc::SymbolMap         symbols;

    symbols[mangle("druk_jit_value_nil")]  = {llvm::orc::ExecutorAddr::fromPtr(&druk_jit_value_nil),
                                              llvm::JITSymbolFlags::Exported};
    symbols[mangle("druk_jit_value_bool")] = {
        llvm::orc::ExecutorAddr::fromPtr(&druk_jit_value_bool), llvm::JITSymbolFlags::Exported};
    symbols[mangle("druk_jit_value_int")] = {llvm::orc::ExecutorAddr::fromPtr(&druk_jit_value_int),
                                             llvm::JITSymbolFlags::Exported};
    symbols[mangle("druk_jit_value_function")] = {
        llvm::orc::ExecutorAddr::fromPtr(&druk_jit_value_function), llvm::JITSymbolFlags::Exported};
    symbols[mangle("druk_jit_add")]        = {llvm::orc::ExecutorAddr::fromPtr(&druk_jit_add),
                                              llvm::JITSymbolFlags::Exported};
    symbols[mangle("druk_jit_subtract")]   = {llvm::orc::ExecutorAddr::fromPtr(&druk_jit_subtract),
                                              llvm::JITSymbolFlags::Exported};
    symbols[mangle("druk_jit_multiply")]   = {llvm::orc::ExecutorAddr::fromPtr(&druk_jit_multiply),
                                              llvm::JITSymbolFlags::Exported};
    symbols[mangle("druk_jit_divide")]     = {llvm::orc::ExecutorAddr::fromPtr(&druk_jit_divide),
                                              llvm::JITSymbolFlags::Exported};
    symbols[mangle("druk_jit_negate")]     = {llvm::orc::ExecutorAddr::fromPtr(&druk_jit_negate),
                                              llvm::JITSymbolFlags::Exported};
    symbols[mangle("druk_jit_equal")]      = {llvm::orc::ExecutorAddr::fromPtr(&druk_jit_equal),
                                              llvm::JITSymbolFlags::Exported};
    symbols[mangle("druk_jit_less")]       = {llvm::orc::ExecutorAddr::fromPtr(&druk_jit_less),
                                              llvm::JITSymbolFlags::Exported};
    symbols[mangle("druk_jit_less_equal")] = {llvm::orc::ExecutorAddr::fromPtr(&druk_jit_less_equal),
                                              llvm::JITSymbolFlags::Exported};
    symbols[mangle("druk_jit_greater")]    = {llvm::orc::ExecutorAddr::fromPtr(&druk_jit_greater),
                                              llvm::JITSymbolFlags::Exported};
    symbols[mangle("druk_jit_greater_equal")] = {llvm::orc::ExecutorAddr::fromPtr(&druk_jit_greater_equal),
                                                 llvm::JITSymbolFlags::Exported};
    symbols[mangle("druk_jit_not")]        = {llvm::orc::ExecutorAddr::fromPtr(&druk_jit_not),
                                              llvm::JITSymbolFlags::Exported};
    symbols[mangle("druk_jit_get_global")] = {
        llvm::orc::ExecutorAddr::fromPtr(&druk_jit_get_global), llvm::JITSymbolFlags::Exported};
    symbols[mangle("druk_jit_define_global")] = {
        llvm::orc::ExecutorAddr::fromPtr(&druk_jit_define_global), llvm::JITSymbolFlags::Exported};
    symbols[mangle("druk_jit_set_global")] = {
        llvm::orc::ExecutorAddr::fromPtr(&druk_jit_set_global), llvm::JITSymbolFlags::Exported};
    symbols[mangle("druk_jit_build_array")] = {
        llvm::orc::ExecutorAddr::fromPtr(&druk_jit_build_array), llvm::JITSymbolFlags::Exported};

    llvm::cantFail(jd.define(llvm::orc::absoluteSymbols(std::move(symbols))));
}

}  // namespace druk::codegen

#endif  // DRUK_HAVE_LLVM
