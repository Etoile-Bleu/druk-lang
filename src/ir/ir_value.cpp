#include "druk/ir/ir_value.h"

#include <iostream>
#include <sstream>

#include "druk/ir/ir_type.h"


namespace druk::ir
{

ConstantInt::ConstantInt(int64_t value, std::shared_ptr<Type> type)
    : value_(value), type_(std::move(type))
{
}

std::string ConstantInt::toString() const
{
    std::stringstream ss;
    ss << value_;
    return ss.str();
}

ConstantBool::ConstantBool(bool value, std::shared_ptr<Type> type)
    : value_(value), type_(std::move(type))
{
}

std::string ConstantBool::toString() const
{
    return value_ ? "true" : "false";
}

Parameter::Parameter(const std::string& name, std::shared_ptr<Type> type, size_t index)
    : type_(std::move(type)), index_(index)
{
    setName(name);
}

std::string Parameter::toString() const
{
    return "%" + getName();
}

ConstantString::ConstantString(std::string value, std::shared_ptr<Type> type)
    : value_(std::move(value)), type_(std::move(type))
{
}

std::string ConstantString::toString() const
{
    return "\"" + value_ + "\"";
}

ConstantNil::ConstantNil(std::shared_ptr<Type> type) : type_(std::move(type))
{
}

std::string ConstantNil::toString() const
{
    return "nil";
}

}  // namespace druk::ir
