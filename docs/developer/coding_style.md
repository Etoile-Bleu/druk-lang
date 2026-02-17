# Druk Coding Style Guide

## Naming Conventions
- Classes/Structs: `PascalCase`
- Functions/Methods: `camelCase`
- Variables: `camelCase`
- Private members: trailing `_`
- Constants: `kPascalCase`
- Namespaces: `snake_case`

## File Organization
- Max 300 lines per .cpp file
- Max 200 lines per .h file
- Max 50 lines per function
- One class per file

## Comments
- Use Doxygen style for public APIs
- Inline comments for complex logic only
- Self-documenting code preferred

## Memory Management
- Use smart pointers (`std::unique_ptr`, `std::shared_ptr`)
- Avoid raw `new`/`delete`
- Use RAII pattern

## Error Handling
- Use exceptions for exceptional cases
- Return `std::optional` or `std::expected` for expected failures
- Always document what exceptions can be thrown
