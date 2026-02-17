// Collection opcodes implementation for VM
// Included directly into vm.cpp run() function

case OpCode::BuildArray : {
  {
    uint8_t count = READ_BYTE();
    auto array = std::make_shared<ObjArray>();
    array->elements.resize(count);
    for (int i = count - 1; i >= 0; --i) {
      array->elements[static_cast<size_t>(i)] = pop();
    }
    push(Value(array));
  }
  break;
}

case OpCode::Index : {
  {
    Value indexVal = pop();
    Value arrayVal = pop();
    if (!indexVal.isInt()) {
      frame_->ip = ip;
      runtimeError("Array index must be an integer.");
      return InterpretResult::RuntimeError;
    }
    if (!arrayVal.isArray()) {
      frame_->ip = ip;
      runtimeError("Can only index arrays.");
      return InterpretResult::RuntimeError;
    }
    int64_t index = indexVal.asInt();
    auto array = arrayVal.asArray();
    if (index < 0 || index >= static_cast<int64_t>(array->elements.size())) {
      frame_->ip = ip;
      runtimeError("Array index out of bounds.");
      return InterpretResult::RuntimeError;
    }
    push(array->elements[static_cast<size_t>(index)]);
  }
  break;
}

case OpCode::IndexSet : {
  {
    Value value = pop();
    Value indexVal = pop();
    Value arrayVal = pop();
    if (!indexVal.isInt()) {
      frame_->ip = ip;
      runtimeError("Array index must be an integer.");
      return InterpretResult::RuntimeError;
    }
    if (!arrayVal.isArray()) {
      frame_->ip = ip;
      runtimeError("Can only index arrays.");
      return InterpretResult::RuntimeError;
    }
    int64_t index = indexVal.asInt();
    auto array = arrayVal.asArray();
    if (index < 0 || index >= static_cast<int64_t>(array->elements.size())) {
      frame_->ip = ip;
      runtimeError("Array index out of bounds.");
      return InterpretResult::RuntimeError;
    }
    array->elements[static_cast<size_t>(index)] = value;
    push(value);
  }
  break;
}

case OpCode::BuildStruct : {
  {
    uint8_t fieldCount = READ_BYTE();
    auto obj = std::make_shared<ObjStruct>();
    for (int i = fieldCount - 1; i >= 0; --i) {
      Value value = pop();
      Value nameVal = pop();
      if (!nameVal.isString()) {
        frame_->ip = ip;
        runtimeError("Struct field name must be a string.");
        return InterpretResult::RuntimeError;
      }
      obj->fields[std::string(nameVal.asString())] = value;
    }
    push(Value(obj));
  }
  break;
}
