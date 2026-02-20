    // Built-in functions implementation for VM
    // Included directly into vm.cpp run() function

case OpCode::Len:
{
    {
        Value val = pop();
        if (val.isArray())
        {
            push(Value(static_cast<int64_t>(val.asGcArray()->elements.size())));
        }
        else if (val.isStruct())
        {
            push(Value(static_cast<int64_t>(val.asGcStruct()->fields.size())));
        }
        else
        {
            frame_->ip = ip;
            runtimeError("len() requires array or struct.");
            return InterpretResult::RuntimeError;
        }
    }
    break;
}

case OpCode::Push:
{
    {
        Value element = pop();
        Value arrayVal = pop();
        if (!arrayVal.isArray())
        {
            frame_->ip = ip;
            runtimeError("push() requires array as first argument.");
            return InterpretResult::RuntimeError;
        }
        arrayVal.asGcArray()->elements.push_back(element);
        push(Value());
    }
    break;
}

case OpCode::PopArray:
{
    {
        Value arrayVal = pop();
        if (!arrayVal.isArray())
        {
            frame_->ip = ip;
            runtimeError("pop() requires array.");
            return InterpretResult::RuntimeError;
        }
        auto* arr = arrayVal.asGcArray();
        if (arr->elements.empty())
        {
            frame_->ip = ip;
            runtimeError("Cannot pop from empty array.");
            return InterpretResult::RuntimeError;
        }
        Value element = arr->elements.back();
        arr->elements.pop_back();
        push(element);
    }
    break;
}

case OpCode::TypeOf:
{
    {
        Value val = pop();
        if (val.isInt())
            push(Value(storeString("int")));
        else if (val.isBool())
            push(Value(storeString("bool")));
        else if (val.isString())
            push(Value(storeString("string")));
        else if (val.isArray())
            push(Value(storeString("array")));
        else if (val.isStruct())
            push(Value(storeString("struct")));
        else
            push(Value(storeString("nil")));
    }
    break;
}

case OpCode::Keys:
{
    {
        Value objVal = pop();
        if (!objVal.isStruct())
        {
            frame_->ip = ip;
            runtimeError("keys() requires a struct.");
            return InterpretResult::RuntimeError;
        }
        auto* obj = objVal.asGcStruct();
        auto* keys = gc::GcHeap::get().alloc<gc::GcArray>();
        keys->elements.reserve(obj->fields.size());
        for (const auto& pair : obj->fields)
        {
            keys->elements.push_back(Value(storeString(pair.first)));
        }
        push(Value(keys));
    }
    break;
}

case OpCode::Values:
{
    {
        Value objVal = pop();
        if (!objVal.isStruct())
        {
            frame_->ip = ip;
            runtimeError("values() requires a struct.");
            return InterpretResult::RuntimeError;
        }
        auto* obj = objVal.asGcStruct();
        auto* values = gc::GcHeap::get().alloc<gc::GcArray>();
        values->elements.reserve(obj->fields.size());
        for (const auto& pair : obj->fields)
        {
            values->elements.push_back(pair.second);
        }
        push(Value(values));
    }
    break;
}

case OpCode::Contains:
{
    {
        Value needle = pop();
        Value haystack = pop();
        if (haystack.isArray())
        {
            auto* arr = haystack.asGcArray();
            bool found = false;
            for (const auto& v : arr->elements)
            {
                if (v == needle)
                {
                    found = true;
                    break;
                }
            }
            push(Value(found));
        }
        else if (haystack.isStruct())
        {
            auto* obj = haystack.asGcStruct();
            if (!needle.isString())
                push(Value(false));
            else
                push(Value(obj->fields.find(std::string(needle.asString())) != obj->fields.end()));
        }
        else
        {
            frame_->ip = ip;
            runtimeError("contains() requires array or struct.");
            return InterpretResult::RuntimeError;
        }
    }
    break;
}

case OpCode::Input:
{
    {
        std::string line;
        if (!std::getline(std::cin, line))
            push(Value());
        else
            push(Value(storeString(std::move(line))));
    }
    break;
}
