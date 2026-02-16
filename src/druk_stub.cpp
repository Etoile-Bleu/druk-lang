// Stub exécutable pour les programmes Druk compilés
// Ce programme est compilé une seule fois et réutilisé pour tous les .exe générés
#ifdef _WIN32
#include <windows.h>
#endif

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <vector>
#include <memory>

// Déclarations externes de l'API C
extern "C" {
  void* druk_chunk_create();
  void druk_chunk_destroy(void* chunk);
  void druk_chunk_deserialize(void* chunk, const uint8_t* buffer, size_t size);
  void* druk_vm_create();
  void druk_vm_destroy(void* vm);
    void druk_vm_set_args(void* vm, int argc, const char** argv);
  void* druk_function_create();
  void druk_function_destroy(void* func);
  void druk_function_set_chunk(void* func, void* chunk);
  void druk_function_set_name(void* func, const char* name);
  int druk_vm_interpret(void* vm, void* func);
}

// Marqueur magique pour identifier le bytecode embarqué
static const char MAGIC[] = "DRUK_BYTECODE_V1";
static const size_t MAGIC_LEN = 16;

std::vector<uint8_t> load_embedded_bytecode() {
    std::vector<uint8_t> bytecode;
    
#ifdef _WIN32
    // Obtenir le chemin de l'exécutable actuel
    char exe_path[MAX_PATH];
    GetModuleFileNameA(NULL, exe_path, MAX_PATH);
    
    // Ouvrir le fichier exe
    FILE* file = fopen(exe_path, "rb");
    if (!file) {
        fprintf(stderr, "Error: Cannot open executable\n");
        return bytecode;
    }
    
    // Aller à la fin du fichier
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    
    // Chercher le marqueur magique depuis la fin
    // Le bytecode est stocké comme: [bytecode_data][size:4bytes][MAGIC:16bytes]
    fseek(file, -(long)(MAGIC_LEN + 4), SEEK_END);
    
    char magic_check[MAGIC_LEN + 1] = {0};
    uint32_t bytecode_size = 0;
    
    // Lire la taille
    fread(&bytecode_size, 4, 1, file);
    // Lire le magic
    fread(magic_check, 1, MAGIC_LEN, file);
    
    if (memcmp(magic_check, MAGIC, MAGIC_LEN) == 0) {
        // Bytecode trouvé !
        long bytecode_start = file_size - MAGIC_LEN - 4 - bytecode_size;
        fseek(file, bytecode_start, SEEK_SET);
        
        bytecode.resize(bytecode_size);
        fread(bytecode.data(), 1, bytecode_size, file);
    } else {
        fprintf(stderr, "Error: No embedded bytecode found\n");
        fprintf(stderr, "This executable must be generated with 'druk compile'\n");
    }
    
    fclose(file);
#else
    // Pour Linux/Unix, approche similaire
    FILE* file = fopen("/proc/self/exe", "rb");
    if (!file) {
        fprintf(stderr, "Error: Cannot open executable\n");
        return bytecode;
    }
    
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, -(long)(MAGIC_LEN + 4), SEEK_END);
    
    char magic_check[MAGIC_LEN + 1] = {0};
    uint32_t bytecode_size = 0;
    
    fread(&bytecode_size, 4, 1, file);
    fread(magic_check, 1, MAGIC_LEN, file);
    
    if (memcmp(magic_check, MAGIC, MAGIC_LEN) == 0) {
        long bytecode_start = file_size - MAGIC_LEN - 4 - bytecode_size;
        fseek(file, bytecode_start, SEEK_SET);
        
        bytecode.resize(bytecode_size);
        fread(bytecode.data(), 1, bytecode_size, file);
    }
    
    fclose(file);
#endif
    
    return bytecode;
}

int main(int argc, char** argv) {
#ifdef _WIN32
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
#endif

    // Charger le bytecode embarqué
    auto bytecode = load_embedded_bytecode();
    if (bytecode.empty()) {
        return 1;
    }

    // Créer le chunk et désérialiser
    void* chunk = druk_chunk_create();
    druk_chunk_deserialize(chunk, bytecode.data(), bytecode.size());

    // Créer la fonction
    void* func = druk_function_create();
    druk_function_set_chunk(func, chunk);
    druk_function_set_name(func, "script");

    // Exécuter
    void* vm = druk_vm_create();
    druk_vm_set_args(vm, argc, const_cast<const char**>(argv));
    int result = druk_vm_interpret(vm, func);

    // Cleanup
    druk_vm_destroy(vm);
    druk_function_destroy(func);
    druk_chunk_destroy(chunk);

    return result;
}
