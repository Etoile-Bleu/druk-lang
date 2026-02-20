#include <iostream>
#include <cstdlib>

#include "druk/codegen/jit/jit_runtime.h"

extern "C" {

void druk_jit_panic_unwrap()
{
    std::cerr << "Runtime Error: Attempted to forcefully unwrap a nil value ('ཅི་མེད') using the '!' operator." << std::endl;
    std::abort();
}

}
