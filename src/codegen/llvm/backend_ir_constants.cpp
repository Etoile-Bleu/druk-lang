#ifdef DRUK_HAVE_LLVM

#include "druk/codegen/llvm/llvm_backend.h"

#include <llvm/IR/Constants.h>
#include <llvm/IR/GlobalVariable.h>

#include "druk/codegen/core/value.h"
#include "druk/ir/ir_value.h"

namespace druk::codegen
{

llvm::Value* LLVMBackend::get_llvm_value(ir::Value* value)
{
    if (ctx_->ir_values.count(value))
        return ctx_->ir_values[value];

    llvm::Type*       i8_ty      = llvm::Type::getInt8Ty(*ctx_->context);
    llvm::Type*       i64_ty     = llvm::Type::getInt64Ty(*ctx_->context);
    llvm::ArrayType*  padding_ty = llvm::ArrayType::get(i8_ty, 7);
    llvm::StructType* packed_value_ty =
        llvm::StructType::get(*ctx_->context, {i8_ty, padding_ty, i64_ty, i64_ty}, false);

    if (auto* cInt = dynamic_cast<ir::ConstantInt*>(value))
    {
        llvm::Value* alloc = create_entry_alloca(packed_value_ty, "const_int");

        llvm::Value* typePtr = ctx_->builder->CreateStructGEP(packed_value_ty, alloc, 0);
        ctx_->builder->CreateStore(
            llvm::ConstantInt::get(i8_ty, static_cast<uint8_t>(ValueType::Int)), typePtr);

        llvm::Value* dataPtr = ctx_->builder->CreateStructGEP(packed_value_ty, alloc, 2);
        ctx_->builder->CreateStore(llvm::ConstantInt::get(i64_ty, cInt->getValue()), dataPtr);

        llvm::Value* extraPtr = ctx_->builder->CreateStructGEP(packed_value_ty, alloc, 3);
        ctx_->builder->CreateStore(llvm::ConstantInt::get(i64_ty, 0), extraPtr);

        return alloc;
    }
    else if (auto* cBool = dynamic_cast<ir::ConstantBool*>(value))
    {
        llvm::Value* alloc = create_entry_alloca(packed_value_ty, "const_bool");

        llvm::Value* typePtr = ctx_->builder->CreateStructGEP(packed_value_ty, alloc, 0);
        ctx_->builder->CreateStore(
            llvm::ConstantInt::get(i8_ty, static_cast<uint8_t>(ValueType::Bool)), typePtr);

        llvm::Value* dataPtr = ctx_->builder->CreateStructGEP(packed_value_ty, alloc, 2);
        ctx_->builder->CreateStore(llvm::ConstantInt::get(i64_ty, cBool->getValue() ? 1 : 0),
                                   dataPtr);

        llvm::Value* extraPtr = ctx_->builder->CreateStructGEP(packed_value_ty, alloc, 3);
        ctx_->builder->CreateStore(llvm::ConstantInt::get(i64_ty, 0), extraPtr);

        return alloc;
    }
    else if (auto* cStr = dynamic_cast<ir::ConstantString*>(value))
    {
        llvm::Value* alloc = create_entry_alloca(packed_value_ty, "const_str");

        llvm::Constant* strConst =
            llvm::ConstantDataArray::getString(*ctx_->context, cStr->getValue());
        llvm::GlobalVariable* strGlobal =
            new llvm::GlobalVariable(*ctx_->module, strConst->getType(), true,
                                     llvm::GlobalValue::PrivateLinkage, strConst, ".str");

        llvm::Value* typePtr = ctx_->builder->CreateStructGEP(packed_value_ty, alloc, 0);
        ctx_->builder->CreateStore(
            llvm::ConstantInt::get(i8_ty, static_cast<uint8_t>(ValueType::String)), typePtr);

        llvm::Value* dataPtr = ctx_->builder->CreateStructGEP(packed_value_ty, alloc, 2);
        llvm::Value* strPtr =
            ctx_->builder->CreateBitCast(strGlobal, llvm::PointerType::getUnqual(*ctx_->context));
        llvm::Value* strInt = ctx_->builder->CreatePtrToInt(strPtr, i64_ty);
        ctx_->builder->CreateStore(strInt, dataPtr);

        llvm::Value* extraPtr = ctx_->builder->CreateStructGEP(packed_value_ty, alloc, 3);
        ctx_->builder->CreateStore(llvm::ConstantInt::get(i64_ty, cStr->getValue().size()),
                                   extraPtr);

        return alloc;
    }

    return nullptr;
}

}  // namespace druk::codegen

#endif  // DRUK_HAVE_LLVM
