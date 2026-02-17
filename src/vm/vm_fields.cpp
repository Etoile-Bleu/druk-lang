// Field access opcodes implementation for VM
// Included directly into vm.cpp run() function

case OpCode::GetField: {
  {
    uint8_t nameIndex = READ_BYTE();
    Value objVal = pop();
    if (!objVal.isStruct()) {
      frame_->ip = ip;
      runtimeError("Can only access fields on structs.");
      return InterpretResult::RuntimeError;
    }
    auto obj = objVal.asStruct();
    Value nameConstant = frame_->function->chunk.constants()[nameIndex];
    if (!nameConstant.isString()) {
      frame_->ip = ip;
      runtimeError("Field name must be a string.");
      return InterpretResult::RuntimeError;
    }
    std::string name(nameConstant.asString());
    auto it = obj->fields.find(name);
    if (it == obj->fields.end()) {
      frame_->ip = ip;
      runtimeError("Undefined field '%s'.", name.c_str());
      return InterpretResult::RuntimeError;
    }
    push(it->second);
  }
  break;
}

case OpCode::SetField: {
  {
    uint8_t nameIndex = READ_BYTE();
    Value value = pop();
    Value objVal = pop();
    if (!objVal.isStruct()) {
      frame_->ip = ip;
      runtimeError("Can only set fields on structs.");
      return InterpretResult::RuntimeError;
    }
    auto obj = objVal.asStruct();
    Value nameConstant = frame_->function->chunk.constants()[nameIndex];
    if (!nameConstant.isString()) {
      frame_->ip = ip;
      runtimeError("Field name must be a string.");
      return InterpretResult::RuntimeError;
    }
    obj->fields[std::string(nameConstant.asString())] = value;
    push(value);
  }
  break;
}
