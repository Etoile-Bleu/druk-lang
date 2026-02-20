// C API wrapper for embedding Druk in standalone executables
#include <cstring>
#include <string>
#include <vector>

#include "druk/codegen/core/chunk.h"
#include "druk/codegen/core/obj.h"
#include "druk/codegen/core/value.h"
#include "druk/gc/gc_heap.h"
#include "druk/gc/types/gc_string.h"
#include "druk/vm/vm.hpp"

using namespace druk;
using namespace druk::codegen;
using namespace druk::vm;

extern "C"
{
    void* druk_chunk_create()
    {
        return new Chunk();
    }

    void druk_chunk_destroy(void* chunk)
    {
        delete static_cast<Chunk*>(chunk);
    }

    void druk_chunk_write(void* chunk, uint8_t byte, uint32_t line)
    {
        static_cast<Chunk*>(chunk)->write(byte, static_cast<int>(line));
    }

    void* druk_vm_create()
    {
        return new VM();
    }

    void druk_vm_destroy(void* vm)
    {
        delete static_cast<VM*>(vm);
    }

    void druk_vm_set_args(void* vm, int argc, const char** argv)
    {
        auto                     vm_ptr = static_cast<VM*>(vm);
        std::vector<std::string> args;
        args.reserve(static_cast<size_t>(argc));
        for (int i = 0; i < argc; ++i)
        {
            args.emplace_back(argv[i] ? argv[i] : "");
        }
        vm_ptr->set_args(args);
    }

    void* druk_function_create()
    {
        return new ObjFunction();
    }

    void druk_function_set_chunk(void* func, void* chunk)
    {
        auto* func_ptr  = static_cast<ObjFunction*>(func);
        auto* chunk_ptr = static_cast<Chunk*>(chunk);
        func_ptr->chunk = *chunk_ptr;
    }

    void druk_function_set_name(void* func, const char* name)
    {
        static_cast<ObjFunction*>(func)->name = name;
    }

    int druk_vm_interpret(void* vm, void* func)
    {
        try
        {
            auto* vm_ptr   = static_cast<VM*>(vm);
            auto* func_ptr = static_cast<ObjFunction*>(func);
            return static_cast<int>(vm_ptr->interpret(func_ptr));
        }
        catch (const std::exception& e)
        {
            fprintf(stderr, "Exception during interpret: %s\n", e.what());
            return 1;
        }
        catch (...)
        {
            fprintf(stderr, "Unknown exception during interpret\n");
            return 1;
        }
    }

    void druk_function_destroy(void* func)
    {
        delete static_cast<ObjFunction*>(func);
    }

    size_t druk_chunk_serialize_size(void* chunk)
    {
        auto        chunk_ptr = static_cast<Chunk*>(chunk);
        const auto& code      = chunk_ptr->code();
        const auto& constants = chunk_ptr->constants();
        const auto& lines     = chunk_ptr->lines();

        size_t size = 16;
        size += code.size();
        size += lines.size() * sizeof(int);
        for (const auto& constant : constants)
        {
            size += 1;
            if (constant.isString())
            {
                auto str = constant.asString();
                size += 4 + str.size();
            }
            else if (constant.isInt())
            {
                size += 8;
            }
            else if (constant.isBool())
            {
                size += 1;
            }
            else if (constant.isFunction())
            {
                size += 0;
            }
            else
            {
                size += 1;
            }
        }
        return size;
    }

    void druk_chunk_serialize(void* chunk, uint8_t* buffer)
    {
        auto        chunk_ptr = static_cast<Chunk*>(chunk);
        const auto& code      = chunk_ptr->code();
        const auto& constants = chunk_ptr->constants();
        const auto& lines     = chunk_ptr->lines();

        uint8_t* ptr = buffer;
        memcpy(ptr, "CHNK", 4);
        ptr += 4;

        uint32_t code_size      = (uint32_t)code.size();
        uint32_t constants_size = (uint32_t)constants.size();
        uint32_t lines_size     = (uint32_t)lines.size();

        memcpy(ptr, &code_size, 4);
        ptr += 4;
        memcpy(ptr, &constants_size, 4);
        ptr += 4;
        memcpy(ptr, &lines_size, 4);
        ptr += 4;

        memcpy(ptr, code.data(), code.size());
        ptr += code.size();
        memcpy(ptr, lines.data(), lines.size() * sizeof(int));
        ptr += lines.size() * sizeof(int);

        for (const auto& constant : constants)
        {
            if (constant.isString())
            {
                *ptr++            = 1;
                auto     str      = constant.asString();
                uint32_t str_size = (uint32_t)str.size();
                memcpy(ptr, &str_size, 4);
                ptr += 4;
                memcpy(ptr, str.data(), str.size());
                ptr += str.size();
            }
            else if (constant.isInt())
            {
                *ptr++      = 2;
                int64_t val = constant.asInt();
                memcpy(ptr, &val, 8);
                ptr += 8;
            }
            else if (constant.isBool())
            {
                *ptr++ = 3;
                *ptr++ = constant.asBool() ? 1 : 0;
            }
            else if (constant.isFunction())
            {
                *ptr++ = 4;
            }
            else
            {
                *ptr++ = 0;
            }
        }
    }

    void druk_chunk_deserialize(void* chunk, const uint8_t* buffer, size_t)
    {
        auto           chunk_ptr = static_cast<Chunk*>(chunk);
        const uint8_t* ptr       = buffer;
        if (memcmp(ptr, "CHNK", 4) != 0)
            return;
        ptr += 4;

        uint32_t code_size, constants_size, lines_size;
        memcpy(&code_size, ptr, 4);
        ptr += 4;
        memcpy(&constants_size, ptr, 4);
        ptr += 4;
        memcpy(&lines_size, ptr, 4);
        ptr += 4;

        for (uint32_t i = 0; i < code_size; i++) chunk_ptr->write(*ptr++, 0);

        auto& lines = const_cast<std::vector<int>&>(chunk_ptr->lines());
        lines.resize(lines_size);
        memcpy(lines.data(), ptr, lines_size * sizeof(int));
        ptr += lines_size * sizeof(int);

        for (uint32_t i = 0; i < constants_size; i++)
        {
            uint8_t type = *ptr++;
            if (type == 1)
            {
                uint32_t str_size;
                memcpy(&str_size, ptr, 4);
                ptr += 4;
                auto* gs =
                    gc::GcHeap::get().alloc<gc::GcString>(std::string((const char*)ptr, str_size));
                ptr += str_size;
                chunk_ptr->addConstant(Value(gs));
            }
            else if (type == 2)
            {
                int64_t val;
                memcpy(&val, ptr, 8);
                ptr += 8;
                chunk_ptr->addConstant(Value(val));
            }
            else if (type == 3)
            {
                bool val = (*ptr++) != 0;
                chunk_ptr->addConstant(Value(val));
            }
            else if (type == 4)
            {
            }
            else
            {
                chunk_ptr->addConstant(Value());
            }
        }
    }

}  // extern "C"
