// Built-in functions implementation for VM
// Included directly into vm.cpp run() function

case OpCode::Len: {
  {
    Value val = pop();
    if (val.isArray()) {
      push(Value(static_cast<int64_t>(val.asArray()->elements.size())));
    } else if (val.isStruct()) {
      push(Value(static_cast<int64_t>(val.asStruct()->fields.size())));
    } else {
      frame_->ip = ip;
      runtimeError("len() requires array or struct.");
      return InterpretResult::RuntimeError;
    }
  }
  break;
}

case OpCode::Push: {
  {
    Value element = pop();
    Value arrayVal = pop();
    if (!arrayVal.isArray()) {
      frame_->ip = ip;
      runtimeError("push() requires array as first argument.");
      return InterpretResult::RuntimeError;
    }
    arrayVal.asArray()->elements.push_back(element);
    push(Value()); 
  }
  break;
}

case OpCode::PopArray: {
  {
    Value arrayVal = pop();
    if (!arrayVal.isArray()) {
      frame_->ip = ip;
      runtimeError("pop() requires array.");
      return InterpretResult::RuntimeError;
    }
    auto arr = arrayVal.asArray();
    if (arr->elements.empty()) {
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

case OpCode::TypeOf: {
  {
    Value val = pop();
    if (val.isInt()) push(Value(std::string_view("int")));
    else if (val.isBool()) push(Value(std::string_view("bool")));
    else if (val.isString()) push(Value(std::string_view("string")));
    else if (val.isArray()) push(Value(std::string_view("array")));
    else if (val.isStruct()) push(Value(std::string_view("struct")));
    else push(Value(std::string_view("nil")));
  }
  break;
}

case OpCode::Keys: {
  {
    Value objVal = pop();
    if (!objVal.isStruct()) {
      frame_->ip = ip;
      runtimeError("keys() requires a struct.");
      return InterpretResult::RuntimeError;
    }
    auto obj = objVal.asStruct();
    auto keys = std::make_shared<ObjArray>();
    keys->elements.reserve(obj->fields.size());
    for (const auto &pair : obj->fields) {
      keys->elements.push_back(Value(std::string_view(pair.first)));
    }
    push(Value(keys));
  }
  break;
}

case OpCode::Values: {
  {
    Value objVal = pop();
    if (!objVal.isStruct()) {
      frame_->ip = ip;
      runtimeError("values() requires a struct.");
      return InterpretResult::RuntimeError;
    }
    auto obj = objVal.asStruct();
    auto values = std::make_shared<ObjArray>();
    values->elements.reserve(obj->fields.size());
    for (const auto &pair : obj->fields) {
      values->elements.push_back(pair.second);
    }
    push(Value(values));
  }
  break;
}

case OpCode::Contains: {
  {
    Value needle = pop();
    Value haystack = pop();
    if (haystack.isArray()) {
      auto arr = haystack.asArray();
      bool found = false;
      for (const auto &v : arr->elements) {
        if (v == needle) { found = true; break; }
      }
      push(Value(found));
    } else if (haystack.isStruct()) {
      auto obj = haystack.asStruct();
      if (!needle.isString()) push(Value(false));
      else push(Value(obj->fields.find(std::string(needle.asString())) != obj->fields.end()));
    } else {
      frame_->ip = ip;
      runtimeError("contains() requires array or struct.");
      return InterpretResult::RuntimeError;
    }
  }
  break;
}

case OpCode::Input: {
  {
    std::string line;
    if (!std::getline(std::cin, line)) push(Value());
    else push(Value(storeString(std::move(line))));
  }
  break;
}
