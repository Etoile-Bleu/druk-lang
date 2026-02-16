// Built-in functions implementation for VM
// Included directly into vm.cpp run() function

case OpCode::Len: {
  {
    Value val = pop();
    
    if (val.is_array()) {
      auto arr = val.as_array();
      push(Value(static_cast<int64_t>(arr->elements.size())));
    } else if (val.is_struct()) {
      auto obj = val.as_struct();
      push(Value(static_cast<int64_t>(obj->fields.size())));
    } else {
      frame_->ip = ip;
      runtime_error("len() requires array or struct.");
      return InterpretResult::RuntimeError;
    }
  }
  break;
}

case OpCode::Push: {
  {
    Value element = pop();
    Value array_val = pop();
    
    if (!array_val.is_array()) {
      frame_->ip = ip;
      runtime_error("push() requires array as first argument.");
      return InterpretResult::RuntimeError;
    }
    
    auto arr = array_val.as_array();
    arr->elements.push_back(element);
    push(Value()); // Return nil
  }
  break;
}

case OpCode::PopArray: {
  {
    Value array_val = pop();
    
    if (!array_val.is_array()) {
      frame_->ip = ip;
      runtime_error("pop() requires array.");
      return InterpretResult::RuntimeError;
    }
    
    auto arr = array_val.as_array();
    
    if (arr->elements.empty()) {
      frame_->ip = ip;
      runtime_error("Cannot pop from empty array.");
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

    static const std::string kInt = "int";
    static const std::string kBool = "bool";
    static const std::string kString = "string";
    static const std::string kArray = "array";
    static const std::string kStruct = "struct";
    static const std::string kNil = "nil";

    if (val.is_int()) {
      push(Value(std::string_view(kInt)));
    } else if (val.is_bool()) {
      push(Value(std::string_view(kBool)));
    } else if (val.is_string()) {
      push(Value(std::string_view(kString)));
    } else if (val.is_array()) {
      push(Value(std::string_view(kArray)));
    } else if (val.is_struct()) {
      push(Value(std::string_view(kStruct)));
    } else {
      push(Value(std::string_view(kNil)));
    }
  }
  break;
}

case OpCode::Keys: {
  {
    Value obj_val = pop();
    if (!obj_val.is_struct()) {
      frame_->ip = ip;
      runtime_error("keys() requires a struct.");
      return InterpretResult::RuntimeError;
    }

    auto obj = obj_val.as_struct();
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
    Value obj_val = pop();
    if (!obj_val.is_struct()) {
      frame_->ip = ip;
      runtime_error("values() requires a struct.");
      return InterpretResult::RuntimeError;
    }

    auto obj = obj_val.as_struct();
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

    if (haystack.is_array()) {
      auto arr = haystack.as_array();
      bool found = false;
      for (const auto &v : arr->elements) {
        if (v == needle) {
          found = true;
          break;
        }
      }
      push(Value(found));
    } else if (haystack.is_struct()) {
      auto obj = haystack.as_struct();
      if (!needle.is_string()) {
        push(Value(false));
      } else {
        std::string_view key = needle.as_string();
        push(Value(obj->fields.find(std::string(key)) != obj->fields.end()));
      }
    } else {
      frame_->ip = ip;
      runtime_error("contains() requires array or struct.");
      return InterpretResult::RuntimeError;
    }
  }
  break;
}

case OpCode::Input: {
  {
    std::string line;
    if (!std::getline(std::cin, line)) {
      push(Value());
    } else {
      push(Value(store_string(std::move(line))));
    }
  }
  break;
}
