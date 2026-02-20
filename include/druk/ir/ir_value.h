#pragma once

#include <memory>
#include <string>
#include <vector>

namespace druk::ir
{

class Type;

/**
 * @brief Base class for all values in the IR (constants, instruction results, etc.)
 */
class Value
{
   public:
    virtual ~Value() = default;

    virtual std::string           toString() const = 0;
    virtual std::shared_ptr<Type> getType() const  = 0;

    const std::string& getName() const
    {
        return name_;
    }
    void setName(const std::string& name)
    {
        name_ = name;
    }

   protected:
    std::string name_;
};

class Constant : public Value
{
   public:
    virtual ~Constant() = default;
};

class ConstantInt : public Constant
{
   public:
    explicit ConstantInt(int64_t value, std::shared_ptr<Type> type);

    int64_t getValue() const
    {
        return value_;
    }

    std::string           toString() const override;
    std::shared_ptr<Type> getType() const override
    {
        return type_;
    }

   private:
    int64_t               value_;
    std::shared_ptr<Type> type_;
};

class ConstantBool : public Constant
{
   public:
    explicit ConstantBool(bool value, std::shared_ptr<Type> type);

    bool getValue() const
    {
        return value_;
    }

    std::string           toString() const override;
    std::shared_ptr<Type> getType() const override
    {
        return type_;
    }

   private:
    bool                  value_;
    std::shared_ptr<Type> type_;
};

/**
 * @brief Represents a function parameter.
 */
class Parameter : public Value
{
   public:
    Parameter(const std::string& name, std::shared_ptr<Type> type, size_t index);

    std::string           toString() const override;
    std::shared_ptr<Type> getType() const override
    {
        return type_;
    }

    size_t getIndex() const
    {
        return index_;
    }

   private:
    std::shared_ptr<Type> type_;
    size_t                index_;  // Parameter position (0, 1, 2, ...)
};

class ConstantString : public Constant
{
   public:
    explicit ConstantString(std::string value, std::shared_ptr<Type> type);

    const std::string& getValue() const
    {
        return value_;
    }

    std::string           toString() const override;
    std::shared_ptr<Type> getType() const override
    {
        return type_;
    }

   private:
    std::string           value_;
    std::shared_ptr<Type> type_;
};

class ConstantNil : public Constant
{
   public:
    explicit ConstantNil(std::shared_ptr<Type> type);

    std::string           toString() const override;
    std::shared_ptr<Type> getType() const override
    {
        return type_;
    }

   private:
    std::shared_ptr<Type> type_;
};

}  // namespace druk::ir
