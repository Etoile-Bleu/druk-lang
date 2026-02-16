#ifdef _WIN32
#include <windows.h>
#endif

#include "druk/codegen/chunk.hpp"
#include "druk/codegen/generator.hpp"
#include "druk/common/allocator.hpp"
#include "druk/common/error.hpp"
#include "druk/lexer/lexer.hpp"
#include "druk/lexer/unicode.hpp"
#include "druk/parser/core/parser.hpp"
#include "druk/semantic/analyzer.hpp"
#include "druk/vm/vm.hpp"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <cstdlib>
#include <cstdio>
#include <vector>
// Déclarations des fonctions C API pour la sérialisation
extern "C" {
  void* druk_chunk_create();
  size_t druk_chunk_serialize_size(void* chunk);
  void druk_chunk_serialize(void* chunk, uint8_t* buffer);
}
namespace druk {
namespace {

std::string read_file(const std::string &path) {
  std::ifstream file(path, std::ios::binary);
  if (!file.is_open()) {
    std::cerr << "Could not open file: " << path << "\n";
    return "";
  }
  std::stringstream buffer;
  buffer << file.rdbuf();
  return buffer.str();
}

std::vector<std::string> build_args(int argc, char *argv[], int start_index) {
  std::vector<std::string> args;
  if (start_index < 0 || start_index >= argc) {
    return args;
  }
  args.reserve(static_cast<size_t>(argc - start_index));
  for (int i = start_index; i < argc; ++i) {
    args.emplace_back(argv[i] ? argv[i] : "");
  }
  return args;
}

} // namespace
} // namespace druk

int main(int argc, char *argv[]) {
#ifdef _WIN32
  // Set console output to UTF-8
  SetConsoleOutputCP(65001); // CP_UTF8
  SetConsoleCP(65001);
#endif

  if (argc < 2) {
    std::cout << "Druk Language Compiler v0.1.0\n";
    std::cout << "Usage: druk [path]                    (Run script)\n";
    std::cout << "       druk --execute [bytecode]      (Execute bytecode)\n";
    std::cout << "       druk compile [path] -o [exe]   (Compile to executable)\n";
    return 0;
  }

  // Check for bytecode execution mode
  if (std::string(argv[1]) == "--execute") {
    if (argc < 3) {
      std::cerr << "Usage: druk --execute [bytecode]\n";
      return 1;
    }

    // Load and execute bytecode file
    std::ifstream bytecode_file(argv[2], std::ios::binary);
    if (!bytecode_file.is_open()) {
      std::cerr << "Could not open bytecode file: " << argv[2] << "\n";
      return 1;
    }

    druk::Chunk chunk;
    char byte;
    while (bytecode_file.get(byte)) {
      uint8_t ubyte = static_cast<uint8_t>(byte);
      chunk.write(ubyte, 0);
    }
    bytecode_file.close();

    druk::VM vm;
    auto function = std::make_shared<druk::ObjFunction>();
    function->chunk = std::move(chunk);
    function->name = "script";
    vm.set_args(druk::build_args(argc, argv, 2));
    vm.interpret(function);
    return 0;
  }

  // Check for compile mode
  bool compile_mode = false;
  std::string input_file;
  std::string output_file;

  if (std::string(argv[1]) == "compile") {
    if (argc < 4) {
      std::cerr << "Usage: druk compile [path] -o [exe]\n";
      return 1;
    }
    compile_mode = true;
    input_file = argv[2];
    // argv[3] should be "-o"
    if (std::string(argv[3]) != "-o") {
      std::cerr << "Expected '-o' flag\n";
      return 1;
    }
    if (argc < 5) {
      std::cerr << "Expected output file after '-o'\n";
      return 1;
    }
    output_file = argv[4];
  } else {
    input_file = argv[1];
    output_file = "";
  }

  std::string source = druk::read_file(input_file);
  if (source.empty())
    return 1;

  druk::ArenaAllocator arena;
  druk::ErrorReporter errors;
  druk::StringInterner interner(arena);

  // Phase 1: Parsing
  druk::Parser parser(source, arena, interner, errors);
  auto statements = parser.parse();

  if (errors.has_errors()) {
    errors.print(source);
    return 1;
  }

  // Phase 2: Semantic Analysis
  druk::SymbolTable symtab;
  druk::SemanticAnalyzer analyzer(symtab, errors, interner, source);
  analyzer.analyze(statements);

  if (errors.has_errors()) {
    errors.print(source);
    return 1;
  }

  // Phase 3: Code Generation
  druk::Chunk chunk;
  druk::Generator generator(chunk, interner, errors, source);
  generator.generate(statements);

  if (errors.has_errors()) {
    errors.print(source);
    return 1;
  }

  // Phase 4: Compilation to executable (if requested)
  if (compile_mode) {
    // Calculer la taille sérialisée du chunk
    void* chunk_ptr = &chunk;
    size_t chunk_ser_size = druk_chunk_serialize_size(chunk_ptr);
    
    // Allouer le buffer et sérialiser
    std::vector<uint8_t> serialized(chunk_ser_size);
    druk_chunk_serialize(chunk_ptr, serialized.data());
    
    // Trouver le stub exécutable
    std::string stub_path;
    #ifdef _WIN32
      // Le stub est dans le même répertoire que druk.exe
      char buffer[MAX_PATH];
      GetModuleFileNameA(NULL, buffer, MAX_PATH);
      std::string exe_path(buffer);
      size_t last_slash = exe_path.find_last_of("\\/");
      if (last_slash != std::string::npos) {
        stub_path = exe_path.substr(0, last_slash + 1) + "druk-stub.exe";
      } else {
        stub_path = "druk-stub.exe";
      }
    #else
      stub_path = "druk-stub";
    #endif

    // Vérifier que le stub existe
    std::ifstream stub_check(stub_path, std::ios::binary);
    if (!stub_check.is_open()) {
      std::cerr << "Error: Stub executable not found at: " << stub_path << "\n";
      std::cerr << "Please rebuild the project to generate druk-stub.\n";
      return 1;
    }
    stub_check.close();

    // Copier le stub vers le fichier de sortie
    #ifdef _WIN32
      std::string copy_cmd = "copy /Y \"" + stub_path + "\" \"" + output_file + "\" >nul";
    #else
      std::string copy_cmd = "cp \"" + stub_path + "\" \"" + output_file + "\"";
    #endif
    
    if (std::system(copy_cmd.c_str()) != 0) {
      std::cerr << "Error: Failed to copy stub executable\n";
      return 1;
    }

    // Ouvrir le fichier de sortie en mode append binaire
    std::ofstream out(output_file, std::ios::binary | std::ios::app);
    if (!out.is_open()) {
      std::cerr << "Error: Cannot open output file: " << output_file << "\n";
      return 1;
    }

    // Écrire le chunk sérialisé
    out.write(reinterpret_cast<const char*>(serialized.data()), serialized.size());
    
    // Écrire la taille du chunk sérialisé (4 bytes)
    uint32_t size = static_cast<uint32_t>(serialized.size());
    out.write(reinterpret_cast<const char*>(&size), 4);
    
    // Écrire le marqueur magique
    const char magic[] = "DRUK_BYTECODE_V1";
    out.write(magic, 16);
    
    out.close();

    std::cout << "✓ Successfully compiled to " << output_file << "\n";
    std::cout << "  Chunk size: " << serialized.size() << " bytes\n";
    std::cout << "  This is a standalone native executable!\n";
    std::cout << "\nRun it with: " << output_file << "\n";

    return 0;
  }

  // Phase 4: Execution (if not in compile mode)
  druk::VM vm;
  auto function = std::make_shared<druk::ObjFunction>();
  function->chunk = std::move(chunk);
  function->name = "script";
  vm.set_args(druk::build_args(argc, argv, 1));
  vm.interpret(function);

  return 0;
}
