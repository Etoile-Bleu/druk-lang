# Druk Language: Future Features & Roadmap

Druk is designed to be a **Simple, High-Performance, GC-backed** language. We want developers to focus on logic without worrying about manual memory management, while still enjoying the speed of an LLVM-compiled binary.

En s'inspirant des géants modernes comme **Python** (pour l'ergonomie, la productivité et l'écosystème) et **Rust** (pour la sécurité, l'outillage et l'expressivité), voici la liste priorisée des fonctionnalités critiques (Gigantic Features) qui manquent encore à Druk pour devenir un langage de production incontournable.

---

## ⚡ Priorité 1 : Ergonomie & Expressivité (Inspiré de Python)

Pour que Druk soit adopté, il doit être aussi agréable à écrire que Python, tout en tournant à la vitesse du C++.

### 1. Dictionnaires Natifs (HashMaps)
*   **The Goal**: Manipulation de données ultra-rapide.
*   **Feature**: Un type natif `[Key: Value]`.
*   **Pourquoi c'est vital (Python)**: L'ADN de Python repose sur les `dict`. Impossible d'imaginer parser du JSON ou gérer des états complexes sans ça.

### 2. Fonctions de Première Classe & Closures (Lambdas)
*   **The Goal**: Programmation Fonctionnelle.
*   **Feature**: Fonctions anonymes et passage de fonctions en paramètres.
*   **Pourquoi c'est vital (Python/JS)**: Permet des API modernes (callbacks, `map`, `filter`, `reduce`).

### 3. String Interpolation (f-strings)
*   **The Goal**: Formatage de texte lisible.
*   **Feature**: Syntaxe `f"Bonjour {nom}, tu as {calcul()}"`.
*   **Pourquoi c'est vital (Python)**: Les f-strings ont révolutionné la propreté du code Python. Concaténer avec `+` est préhistorique.

### 4. Modules et Imports Natifs (Système de Packages)
*   **The Goal**: Réutilisation du code.
*   **Feature**: Pouvoir diviser son code avec `import utils` ou `from math import sin`.
*   **Pourquoi c'est vital (Python/Rust)**: Un langage sans système de module ne peut pas dépasser le stade de "script d'un seul fichier".

---

## 🛡️ Priorité 2 : Sécurité Typage & Contrôle (Inspiré de Rust)

Druk a un GC, on n'a donc pas besoin du Borrow Checker de Rust. Mais Rust brille par d'autres aspects de sécurité qu'il *faut* voler.

### 1. Null Safety Absolue (Types Optionnels)
*   **The Goal**: Éradiquer la *Billion Dollar Mistake* (NullPointerException).
*   **Feature**: Pas de `null` implicite. Utilisation du type `Option<T>` (ou `?T`).
*   **Pourquoi c'est vital (Rust)**: Dans Rust, le compilateur t'oblige à gérer l'absence de valeur. Si ça compile, ça ne crash pas sur un `null`. Druk doit offrir cette même garantie (grâce au Pattern Matching).

### 2. Gestion des Erreurs Explicite (Result<T, E>)
*   **The Goal**: Fini les Exceptions invisibles.
*   **Feature**: Les fonctions qui peuvent échouer retournent un type `Result` (Succès ou Erreur).
*   **Pourquoi c'est vital (Rust/Go)**: Savoir immédiatement en lisant la signature d'une fonction si elle peut crasher, et forcer le traitement de l'erreur.

### 3. Traits / Interfaces (Polymorphisme sans héritage)
*   **The Goal**: Partage de comportements sûr.
*   **Feature**: Définir des contrats (ex: `Printable`, `Iterable`) au lieu d'utiliser l'héritage objet classique.
*   **Pourquoi c'est vital (Rust)**: C'est plus flexible et plus sûr que l'héritage classique (C++/Java) qui finit en plat de spaghettis.

---

## 🚀 Priorité 3 : Concurrence & Outillage (Inspiré de Go/Rust)

### 1. Concurrence Légère (Green Threads / Goroutines)
*   **The Goal**: Serveurs web ultra-rapides.
*   **Feature**: Mot-clé `འགྲུལ་` (spawn) pour lancer des milliers de tâches sans bloquer l'OS, gérées par un scheduler interne Druk.
*   **Pourquoi c'est vital (Go)**: La concurrence moderne ne doit pas être un cauchemar de mutex et de threads POSIX.

### 2. Cargo pour Druk (Gestionnaire de Paquets Universel)
*   **The Goal**: Tout outillage centralisé.
*   **Feature**: Un outil CLI (ex: `druk-cli`) qui fait TOUT : `druk build`, `druk test`, `druk run`, `druk install json-parser`.
*   **Pourquoi c'est vital (Rust/Cargo)**: Cargo est la raison n°1 pour laquelle l'écosystème Rust est si plaisant. 1 outil, 0 configuration.

### 3. Standard Library Complète (Druk-Std)
*   **The Goal**: 'Batteries Included' (Comme Python).
*   **Feature**: JSON, HTTP, Fichiers, Regex natifs. FFI simple pour appeler du C (libcurl, etc.).

---

## ⚙️ Priorité 4 : Optimisations Bas Niveau LLVM

### 1. SIMD Automatique & Vectorisation
*   **Feature**: Utiliser les types de vecteurs natifs LLVM pour que les opérations sur les tableaux (Array) de Druk volent littéralement.

### 2. JIT Tiering avancé
*   **Feature**: Démarrage instantané via interprétation, bascule automatique sur compilation LLVM ultra-optimisée pour les boucles chaudes (Hot Loops).
