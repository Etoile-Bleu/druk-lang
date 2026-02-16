// Collection opcodes implementation for VM
// Included directly into vm.cpp run() function

OP_BUILD_ARRAY: {
  {
    uint8_t count = READ_BYTE();
    auto array = std::make_shared<ObjArray>();
    array->elements.reserve(count);
    
    // Pop elements in reverse order
    for (int i = count - 1; i >= 0; --i) {
      array->elements.insert(array->elements.begin(), pop());
    }
    
    push(Value(array));
  }
  DISPATCH();
}

OP_INDEX: {
  {
    Value index_val = pop();
    Value array_val = pop();
    
    if (!index_val.is_int()) {
      frame_->ip = ip;
      runtime_error("Array index must be an integer.");
      return InterpretResult::RuntimeError;
    }
    
    if (!array_val.is_array()) {
      frame_->ip = ip;
      runtime_error("Can only index arrays.");
      return InterpretResult::RuntimeError;
    }
    
    int64_t index = index_val.as_int();
    auto array = array_val.as_array();
    
    if (index < 0 || index >= static_cast<int64_t>(array->elements.size())) {
      frame_->ip = ip;
      runtime_error("Array index out of bounds.");
      return InterpretResult::RuntimeError;
    }
    
    push(array->elements[static_cast<size_t>(index)]);
  }
  DISPATCH();
}

OP_INDEX_SET: {
  {
    Value value = pop();
    Value index_val = pop();
    Value array_val = pop();
    
    if (!index_val.is_int()) {
      frame_->ip = ip;
      runtime_error("Array index must be an integer.");
      return InterpretResult::RuntimeError;
    }
    
    if (!array_val.is_array()) {
      frame_->ip = ip;
      runtime_error("Can only index arrays.");
      return InterpretResult::RuntimeError;
    }
    
    int64_t index = index_val.as_int();
    auto array = array_val.as_array();
    
    if (index < 0 || index >= static_cast<int64_t>(array->elements.size())) {
      frame_->ip = ip;
      runtime_error("Array index out of bounds.");
      return InterpretResult::RuntimeError;
    }
    
    array->elements[static_cast<size_t>(index)] = value;
    push(value);
  }
  DISPATCH();
}

OP_BUILD_STRUCT: {
  {
    uint8_t field_count = READ_BYTE();
    auto obj = std::make_shared<ObjStruct>();
    
    // Pop field values and names in reverse order
    for (int i = field_count - 1; i >= 0; --i) {
      Value value = pop();
      Value name_val = pop();
      
      if (!name_val.is_string()) {
        frame_->ip = ip;
        runtime_error("Struct field name must be a string.");
        return InterpretResult::RuntimeError;
      }
      
      std::string name(name_val.as_string());
      obj->fields[name] = value;
    }
    
    push(Value(obj));
  }
  DISPATCH();
}
