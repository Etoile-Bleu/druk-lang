# LLVM JIT Backend pour Druk

## 🚀 Installation Rapide

### Windows

#### Option 1 : Installer LLVM pré-compilé

1. **Télécharger LLVM 17+** :
   ```
   https://github.com/llvm/llvm-project/releases/tag/llvmorg-17.0.6
   ```
   Fichier : `LLVM-17.0.6-win64.exe`

2. **Installer** (cocher "Add LLVM to PATH")

3. **Vérifier l'installation** :
   ```powershell
   llvm-config --version
   ```

#### Option 2 : Chocolatey

```powershell
choco install llvm
```

### Linux

```bash
# Ubuntu/Debian
sudo apt-get install llvm-17 llvm-17-dev

# Fedora
sudo dnf install llvm17 llvm17-devel

# Arch
sudo pacman -S llvm
```

### macOS

```bash
brew install llvm@17
```

## 🔨 Compilation

### Avec LLVM JIT (Recommandé)

```powershell
# Windows
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -G "Visual Studio 17 2022"
cmake --build . --config Release
```

```bash
# Linux/macOS
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

### Sans LLVM (VM uniquement)

Si LLVM n'est pas trouvé, le projet compile automatiquement en mode VM seulement.

## 📊 Benchmarks

### Exécuter les benchmarks

```powershell
# Windows (avec script automatique)
.\run_jit_benchmark.ps1

# Ou manuellement
cd build
.\bin\Release\druk_compare_jit_bench.exe
```

```bash
# Linux/macOS
cd build
./bin/druk_compare_jit_bench
```

### Résultats attendus

**Sans JIT (VM bytecode)** :
```
BM_CPP_Sum           0.097 ns      (~10 milliards ops/s)
BM_Druk_VM_Sum       327 ms        (~3 ops/s)
Facteur: 3.37 MILLIARDS x plus lent
```

**Avec JIT LLVM (Objectif)** :
```
BM_CPP_Sum           0.097 ns      (~10 milliards ops/s)
BM_Druk_JIT_Sum      0.5-5 ns      (~200M-2G ops/s)
Facteur: 5-50x plus lent (acceptable!)
```

## 🎯 Optimisations

### Flags de compilation

Le CMakeLists.txt active automatiquement :
- **O3** : Optimisations maximales
- **march=native** : Instructions CPU spécifiques
- **LTO** : Link-Time Optimization
- **Passes LLVM** : InstCombine, GVN, SimplifyCFG, etc.

### Optimisations futures

1. **Constant folding** dans l'AST (avant bytecode)
2. **Inline de fonctions** petites
3. **Loop unrolling** dans LLVM
4. **Type specialization** (si tous les ints, éviter les checks)
5. **Profile-Guided Optimization (PGO)**

## 🔧 Développement

### Structure du code

```
include/druk/codegen/
  ├── llvm_backend.hpp  # Backend LLVM (bytecode → IR)
  └── llvm_jit.hpp      # JIT Engine (API haut niveau)

src/codegen/
  ├── llvm_backend.cpp  # Implémentation backend
  └── llvm_jit.cpp      # Implémentation JIT
```

### Ajouter un opcode

1. **Ajouter dans `OpCode` enum** (opcode.hpp)
2. **Implémenter dans VM** (vm.cpp)
3. **Implémenter dans LLVM backend** (llvm_backend.cpp)
4. **Ajouter un test**

### Debug

Activer les traces LLVM :

```cpp
// Dans llvm_backend.cpp
ctx_->llvm_function->print(llvm::outs()); // Print IR
llvm::verifyModule(*ctx_->module, &llvm::errs());
```

## ⚠️ Limitations actuelles

1. **Opcodes supportés** : arithmétique basique, boucles, conditions
2. **Types** : Seulement `int64_t` pour l'instant
3. **Appels de fonction** : Pas encore implémentés dans JIT
4. **Collections** : Arrays/structs pas encore supportés

## 📈 Roadmap

- [x] Backend LLVM basique
- [x] Support boucles for/while
- [x] Optimisations LLVM O3
- [ ] Support appels de fonctions
- [ ] Support collections (arrays)
- [ ] Constant folding pré-compilation
- [ ] Cache de code compilé
- [ ] Profile-Guided Optimization

## 🐛 Dépannage

### "LLVM not found"

Vérifier que `LLVM_DIR` pointe vers le bon chemin :
```powershell
$env:LLVM_DIR = "C:\Program Files\LLVM\lib\cmake\llvm"
cmake ..
```

### "Verification failed"

Le bytecode généré est invalide. Activer les traces :
```cpp
#define DEBUG_TRACE_EXECUTION
```

### Performance toujours mauvaise

1. Vérifier que le JIT est utilisé (pas la VM)
2. Vérifier les optimisations : `-O3 -march=native`
3. Profiler avec `perf` (Linux) ou VTune (Intel)
