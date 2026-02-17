#include "druk/vm/vm.hpp"

#include <cmath>
#include <cstdarg>
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>

#include "druk/codegen/core/opcode.h"
#include "druk/lexer/unicode.hpp"


#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wpedantic"
#endif

namespace druk::vm
{

using namespace codegen;

VM::VM()
{
    stack_.resize(kStackMax);
    stackBase_ = stack_.data();
    stackTop_  = stackBase_;
    frames_.reserve(64);
}

VM::~VM() {}

void VM::set_args(const std::vector<std::string>& args)
{
    argvStorage_ = args;

    auto argvArray = std::make_shared<ObjArray>();
    argvArray->elements.reserve(argvStorage_.size());
    for (const auto& s : argvStorage_)
    {
        argvArray->elements.push_back(Value(std::string_view(s)));
    }

    auto set_global = [&](std::string_view name, Value value)
    {
        auto it = globals_.find(name);
        if (it == globals_.end())
        {
            globals_.emplace(name, std::move(value));
            globalsVersion_++;
        }
        else
        {
            it->second = std::move(value);
        }
    };

    set_global("argv", Value(argvArray));
    set_global("argc", Value(static_cast<int64_t>(argvStorage_.size())));
    set_global("ནང་འཇུག་ཐོ་", Value(argvArray));
    set_global("ནང་འཇུག་གྲངས་", Value(static_cast<int64_t>(argvStorage_.size())));
}

InterpretResult VM::interpret(std::shared_ptr<ObjFunction> function)
{
    stackTop_ = stackBase_;
    frames_.clear();
    lastResult_ = Value();

    CallFrame frame;
    frame.function = function.get();
    frame.ip       = function->chunk.code().data();
    frame.slots    = stackBase_;

    push(Value(function));
    frames_.push_back(frame);
    frame_ = &frames_.back();

    return run();
}

void VM::push(Value value)
{
    *stackTop_ = std::move(value);
    ++stackTop_;
}

Value VM::pop()
{
    --stackTop_;
    return std::move(*stackTop_);
}

const Value& VM::peek(int distance) const
{
    return *(stackTop_ - 1 - static_cast<size_t>(distance));
}

void VM::runtimeError(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);

    for (int i = static_cast<int>(frames_.size()) - 1; i >= 0; i--)
    {
        CallFrame*   frame    = &frames_[static_cast<size_t>(i)];
        ObjFunction* function = frame->function;
        size_t instruction    = static_cast<size_t>(frame->ip - function->chunk.code().data() - 1);
        int    line           = function->chunk.lines()[instruction];
        fprintf(stderr, "[line %d] in ", line);
        if (function->name.empty())
        {
            fprintf(stderr, "script\n");
        }
        else
        {
            fprintf(stderr, "%s()\n", function->name.c_str());
        }
    }

    frames_.clear();
    stackTop_ = stackBase_;
}

std::string_view VM::storeString(std::string value)
{
    inputStorage_.push_back(std::move(value));
    return std::string_view(inputStorage_.back());
}

InterpretResult VM::run()
{
    const uint8_t* ip = frame_->ip;

#define READ_BYTE() (*ip++)
#define READ_CONSTANT() (frame_->function->chunk.constants()[READ_BYTE()])
#define READ_SHORT() (ip += 2, (uint16_t)((ip[-2] << 8) | ip[-1]))

#define BINARY_OP(type, op)                            \
    do                                                 \
    {                                                  \
        Value* bptr = (stackTop_ - 1);                 \
        Value* aptr = (stackTop_ - 2);                 \
        if (!aptr->isInt() || !bptr->isInt())          \
        {                                              \
            frame_->ip = ip;                           \
            runtimeError("Operands must be numbers."); \
            return InterpretResult::RuntimeError;      \
        }                                              \
        int64_t b = bptr->asInt();                     \
        int64_t a = aptr->asInt();                     \
        stackTop_ -= 2;                                \
        *stackTop_ = Value(type(a op b));              \
        ++stackTop_;                                   \
    } while (false)

    for (;;)
    {
        OpCode instruction = static_cast<OpCode>(READ_BYTE());

        switch (instruction)
        {
            case OpCode::Return:
            {
                Value  result      = pop();
                Value* returnSlots = frame_->slots;
                frames_.pop_back();
                if (frames_.empty())
                {
                    lastResult_ = result;
                    stackTop_   = stackBase_;
                    push(std::move(result));
                    return InterpretResult::Ok;
                }
                frame_    = &frames_.back();
                stackTop_ = returnSlots;
                push(std::move(result));
                ip = frame_->ip;
                break;
            }

            case OpCode::Constant:
            {
                push(READ_CONSTANT());
                break;
            }
            case OpCode::Nil:
            {
                push(Value());
                break;
            }
            case OpCode::True:
            {
                push(Value(true));
                break;
            }
            case OpCode::False:
            {
                push(Value(false));
                break;
            }
            case OpCode::Pop:
            {
                pop();
                break;
            }

            case OpCode::GetLocal:
            {
                uint8_t slot = READ_BYTE();
                push(frame_->slots[slot]);
                break;
            }

            case OpCode::SetLocal:
            {
                uint8_t slot        = READ_BYTE();
                frame_->slots[slot] = peek(0);
                break;
            }

            case OpCode::GetGlobal:
            {
                Value            nameVal = READ_CONSTANT();
                std::string_view name    = nameVal.asString();
                if (globalCache_.slot && globalCache_.version == globalsVersion_ &&
                    globalCache_.name == name)
                {
                    push(*globalCache_.slot);
                }
                else
                {
                    auto it = globals_.find(name);
                    if (it == globals_.end())
                    {
                        frame_->ip = ip;
                        runtimeError("Undefined variable '%.*s'.", static_cast<int>(name.length()),
                                     name.data());
                        return InterpretResult::RuntimeError;
                    }
                    globalCache_.name    = name;
                    globalCache_.slot    = &it->second;
                    globalCache_.version = globalsVersion_;
                    push(it->second);
                }
                break;
            }

            case OpCode::DefineGlobal:
            {
                Value            nameVal = READ_CONSTANT();
                std::string_view name    = nameVal.asString();
                Value            value   = pop();
                auto             it      = globals_.find(name);
                if (it == globals_.end())
                {
                    auto [new_it, inserted] = globals_.emplace(name, std::move(value));
                    it                      = new_it;
                    if (inserted)
                    {
                        globalsVersion_++;
                    }
                }
                else
                {
                    it->second = std::move(value);
                }
                globalCache_.name    = name;
                globalCache_.slot    = &it->second;
                globalCache_.version = globalsVersion_;
                break;
            }

            case OpCode::SetGlobal:
            {
                Value            nameVal = READ_CONSTANT();
                std::string_view name    = nameVal.asString();
                if (globalCache_.slot && globalCache_.version == globalsVersion_ &&
                    globalCache_.name == name)
                {
                    *globalCache_.slot = peek(0);
                }
                else
                {
                    auto it = globals_.find(name);
                    if (it == globals_.end())
                    {
                        frame_->ip = ip;
                        runtimeError("Undefined variable '%.*s'.", static_cast<int>(name.length()),
                                     name.data());
                        return InterpretResult::RuntimeError;
                    }
                    it->second           = peek(0);
                    globalCache_.name    = name;
                    globalCache_.slot    = &it->second;
                    globalCache_.version = globalsVersion_;
                }
                break;
            }

            case OpCode::Equal:
            {
                Value b = pop();
                Value a = pop();
                push(Value(a == b));
                break;
            }

            case OpCode::Greater:
            {
                BINARY_OP(bool, >);
                break;
            }
            case OpCode::Less:
            {
                BINARY_OP(bool, <);
                break;
            }
            case OpCode::Add:
            {
                BINARY_OP(int64_t, +);
                break;
            }
            case OpCode::Subtract:
            {
                BINARY_OP(int64_t, -);
                break;
            }
            case OpCode::Multiply:
            {
                BINARY_OP(int64_t, *);
                break;
            }

            case OpCode::Divide:
            {
                Value* bptr = (stackTop_ - 1);
                Value* aptr = (stackTop_ - 2);
                if (!aptr->isInt() || !bptr->isInt())
                {
                    frame_->ip = ip;
                    runtimeError("Operands must be numbers.");
                    return InterpretResult::RuntimeError;
                }
                int64_t b = bptr->asInt();
                int64_t a = aptr->asInt();
                if (b == 0)
                {
                    frame_->ip = ip;
                    runtimeError("Division by zero.");
                    return InterpretResult::RuntimeError;
                }
                stackTop_ -= 2;
                *stackTop_ = Value(a / b);
                ++stackTop_;
                break;
            }

            case OpCode::Not:
            {
                Value v = pop();
                if (v.isBool())
                    push(Value(!v.asBool()));
                else if (v.isNil())
                    push(Value(true));
                else
                    push(Value(false));
                break;
            }

            case OpCode::Negate:
            {
                Value* vptr = (stackTop_ - 1);
                if (!vptr->isInt())
                {
                    frame_->ip = ip;
                    runtimeError("Operand must be a number.");
                    return InterpretResult::RuntimeError;
                }
                int64_t v = vptr->asInt();
                --stackTop_;
                *stackTop_ = Value(-v);
                ++stackTop_;
                break;
            }

            case OpCode::Print:
            {
                Value val = pop();
                if (val.isInt())
                    std::cout << ::druk::lexer::unicode::toTibetanNumeral(val.asInt()) << "\n";
                else if (val.isBool())
                    std::cout << (val.asBool() ? "བདེན" : "རྫུན") << "\n";
                else if (val.isString())
                    std::cout << val.asString() << "\n";
                else if (val.isNil())
                    std::cout << "nil\n";
                else if (val.isArray())
                    std::cout << "[array:" << val.asArray()->elements.size() << "]\n";
                else if (val.isStruct())
                    std::cout << "{struct:" << val.asStruct()->fields.size() << "}\n";
                break;
            }

            case OpCode::Jump:
            {
                uint16_t offset = READ_SHORT();
                ip += offset;
                break;
            }

            case OpCode::JumpIfFalse:
            {
                uint16_t     offset  = READ_SHORT();
                const Value& v       = peek(0);
                bool         isFalse = v.isBool() ? !v.asBool() : v.isNil();
                if (isFalse)
                    ip += offset;
                break;
            }

            case OpCode::Loop:
            {
                uint16_t offset = READ_SHORT();
                ip -= offset;
                break;
            }

#include "vm_builtins.cpp"
#include "vm_collections.cpp"
#include "vm_fields.cpp"

            case OpCode::Call:
            {
                uint8_t      argCount = READ_BYTE();
                const Value& callee   = peek(argCount);
                if (!callee.isFunction())
                {
                    frame_->ip = ip;
                    runtimeError("Can only call functions.");
                    return InterpretResult::RuntimeError;
                }

                std::shared_ptr<ObjFunction> function = callee.asFunction();
                if (static_cast<int>(argCount) != function->arity)
                {
                    frame_->ip = ip;
                    runtimeError("Expected %d arguments but got %d.", function->arity,
                                 static_cast<int>(argCount));
                    return InterpretResult::RuntimeError;
                }

                if (frames_.size() == 64)
                {
                    frame_->ip = ip;
                    runtimeError("Stack overflow.");
                    return InterpretResult::RuntimeError;
                }

                frame_->ip = ip;
                CallFrame nextFrame;
                nextFrame.function = function.get();
                nextFrame.ip       = function->chunk.code().data();
                nextFrame.slots    = stackTop_ - argCount - 1;

                frames_.push_back(nextFrame);
                frame_ = &frames_.back();
                ip     = frame_->ip;
                break;
            }
        }
    }

#undef BINARY_OP
#undef READ_BYTE
#undef READ_CONSTANT
#undef READ_SHORT
}

}  // namespace druk::vm
