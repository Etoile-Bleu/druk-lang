// Field access opcodes implementation for VM
// Included directly into vm.cpp run() function

OP_GET_FIELD: {
  {
    uint8_t name_index = READ_BYTE();
    Value obj_val = pop();
    
    if (!obj_val.is_struct()) {
      frame_->ip = ip;
      runtime_error("Can only access fields on structs.");
      return InterpretResult::RuntimeError;
    }
    
    auto obj = obj_val.as_struct();
    Value name_constant = frame_->function->chunk.constants()[name_index];
    
    if (!name_constant.is_string()) {
      frame_->ip = ip;
      runtime_error("Field name must be a string.");
      return InterpretResult::RuntimeError;
    }
    
    std::string name(name_constant.as_string());
    auto it = obj->fields.find(name);
    
    if (it == obj->fields.end()) {
      frame_->ip = ip;
      runtime_error("Undefined field '%s'.", name.c_str());
      return InterpretResult::RuntimeError;
    }
    
    push(it->second);
  }
  DISPATCH();
}

OP_SET_FIELD: {
  {
    uint8_t name_index = READ_BYTE();
    Value value = pop();
    Value obj_val = pop();
    
    if (!obj_val.is_struct()) {
      frame_->ip = ip;
      runtime_error("Can only set fields on structs.");
      return InterpretResult::RuntimeError;
    }
    
    auto obj = obj_val.as_struct();
    Value name_constant = frame_->function->chunk.constants()[name_index];
    
    if (!name_constant.is_string()) {
      frame_->ip = ip;
      runtime_error("Field name must be a string.");
      return InterpretResult::RuntimeError;
    }
    
    std::string name(name_constant.as_string());
    obj->fields[name] = value;
    
    push(value);
  }
  DISPATCH();
}
