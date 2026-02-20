#pragma once

#include <memory>
#include <string>
#include <vector>

namespace druk::ir
{

enum class TypeID
{
    Void,
    Int8,
    Int64,
    Float64,
    Bool,
    Pointer,
    Function,
    Array,
    Struct
};

/**
 * @brief Base class for all IR types.
 */
class Type
{
   public:
    explicit Type(TypeID id) : id_(id) {}
    virtual ~Type() = default;

    TypeID getID() const
    {
        return id_;
    }
    virtual std::string toString() const = 0;

    static std::shared_ptr<Type> getVoidTy();
    static std::shared_ptr<Type> getInt8Ty();
    static std::shared_ptr<Type> getInt64Ty();
    static std::shared_ptr<Type> getFloat64Ty();
    static std::shared_ptr<Type> getBoolTy();

   private:
    TypeID id_;
};

class PointerType : public Type
{
   public:
    explicit PointerType(std::shared_ptr<Type> elementTy)
        : Type(TypeID::Pointer), element_ty_(elementTy)
    {
    }

    std::string toString() const override
    {
        return element_ty_->toString() + "*";
    }
    std::shared_ptr<Type> getElementType() const
    {
        return element_ty_;
    }

   private:
    std::shared_ptr<Type> element_ty_;
};

}  // namespace druk::ir
