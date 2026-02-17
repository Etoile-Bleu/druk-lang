#include "druk/codegen/core/obj.h"
#include "druk/codegen/core/value.h"
#include "druk/codegen/jit/jit_runtime.h"


// Define runtime functions that JIT-compiled code will call.
// These must be exported with C linkage to be easily resolvable.

extern "C"
{
    void LLVMInitializeX86TargetInfo();
    void LLVMInitializeX86Target();
    void LLVMInitializeX86TargetMC();
    void LLVMInitializeX86AsmPrinter();

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
    void druk_jit_greater(const PackedValue* a, const PackedValue* b, PackedValue* out);
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
