#include "druk/ir/ir_type.h"

namespace druk::ir
{

class PrimitiveType : public Type
{
   public:
    PrimitiveType(TypeID id, const std::string& name) : Type(id), name_(name) {}
    std::string toString() const override
    {
        return name_;
    }

   private:
    std::string name_;
};

std::shared_ptr<Type> Type::getVoidTy()
{
    static auto ty = std::make_shared<PrimitiveType>(TypeID::Void, "void");
    return ty;
}

std::shared_ptr<Type> Type::getInt8Ty()
{
    static auto ty = std::make_shared<PrimitiveType>(TypeID::Int8, "i8");
    return ty;
}

std::shared_ptr<Type> Type::getInt64Ty()
{
    static auto ty = std::make_shared<PrimitiveType>(TypeID::Int64, "i64");
    return ty;
}

std::shared_ptr<Type> Type::getFloat64Ty()
{
    static auto ty = std::make_shared<PrimitiveType>(TypeID::Float64, "double");
    return ty;
}

std::shared_ptr<Type> Type::getBoolTy()
{
    static auto ty = std::make_shared<PrimitiveType>(TypeID::Bool, "i1");
    return ty;
}

}  // namespace druk::ir
