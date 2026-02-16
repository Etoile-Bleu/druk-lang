export const translations = {
  en: {
    langName: 'English',
    nav: {
      playground: 'Playground',
      docs: 'Documentation',
      examples: 'Examples',
      language: 'Language'
    },
    hero: {
      title: 'Druk Playground',
      subtitle: 'Dzongkha-first programming • Bhutan-inspired • Web ready',
      run: 'Run (WASM)',
      reset: 'Reset'
    },
    panels: {
      source: 'Source (Dzongkha)',
      status: 'WASM status',
      output: 'Output',
      outputHint: 'Run a program to see output.'
    },
    docs: {
      enTitle: 'Technical Documentation (English)',
      dzTitle: 'ལག་དེབ་ཚན་པ། (Dzongkha)',
      overviewTitle: 'Language Overview',
      overviewBody: 'Druk is a Dzongkha-first programming language with a bytecode VM. Pipeline: Lexer → Parser (AST) → Semantic Analyzer → Codegen → VM.',
      runtimeTitle: 'Runtime Model',
      runtimeItems: [
        'Stack-based bytecode VM with call frames.',
        'Values: int64, bool, string, array, struct, nil.',
        'Arrays are dynamic vectors; structs are key/value maps.'
      ],
      collectionsTitle: 'Collections',
      collectionsItems: [
        'Array literal: [༡, ༢, ༣]',
        'Indexing: arr[༠], assignment: arr[༠] = ༩',
        'Struct literal: {name: "Tenzin", age: ༢༥}',
        'Field access: person.name, assignment: person.age = ༣༠'
      ],
      builtinsTitle: 'Built-ins (Dzongkha)',
      builtinsItems: [
        'ཚད་(x) → length',
        'སྣོན་(arr, v) → push',
        'ཕྱིར་ལེན་(arr) → pop',
        'རིགས་(x) → type',
        'མིང་ཐོ་(struct) → keys',
        'བེད་སྤྱོད་(struct) → values',
        'ཡོད་(container, v) → contains',
        'ནང་འཇུག་() → input line'
      ],
      cliTitle: 'CLI',
      cliItems: [
        'Run: druk path/to/file.druk',
        'Compile: druk compile path/to/file.druk -o out.exe',
        'Execute bytecode: druk --execute file.bytecode'
      ],
      argsTitle: 'Arguments',
      argsBody: 'argv/argc are exposed as globals, plus Dzongkha aliases.',
      argsItems: ['argv, argc', 'ནང་འཇུག་ཐོ་, ནང་འཇུག་གྲངས་'],
      wasmTitle: 'Web/WASM Architecture',
      wasmBody: 'Source (Monaco) → C++ compiler compiled to WASM → Bytecode → VM WASM → Output UI.'
    },
    footer: 'Druk • Bhutan-inspired UI • Framework build • Multilingual'
  },
  fr: {
    langName: 'Français',
    nav: {
      playground: 'Playground',
      docs: 'Documentation',
      examples: 'Exemples',
      language: 'Langue'
    },
    hero: {
      title: 'Druk Playground',
      subtitle: 'Langage dzongkha • Inspiration Bhoutan • Prêt pour le web',
      run: 'Exécuter (WASM)',
      reset: 'Réinitialiser'
    },
    panels: {
      source: 'Source (Dzongkha)',
      status: 'Statut WASM',
      output: 'Sortie',
      outputHint: 'Exécute un programme pour voir la sortie.'
    },
    docs: {
      enTitle: 'Documentation technique (Anglais)',
      dzTitle: 'ལག་དེབ་ཚན་པ། (Dzongkha)',
      overviewTitle: 'Vue d’ensemble',
      overviewBody: 'Druk est un langage Dzongkha avec VM bytecode. Pipeline : Lexer → Parser (AST) → Semantic → Codegen → VM.',
      runtimeTitle: 'Modèle d’exécution',
      runtimeItems: [
        'VM bytecode stack-based avec call frames.',
        'Types : int64, bool, string, array, struct, nil.',
        'Arrays = vecteurs dynamiques, structs = maps clé/valeur.'
      ],
      collectionsTitle: 'Collections',
      collectionsItems: [
        'Array : [༡, ༢, ༣]',
        'Index : arr[༠], assignation : arr[༠] = ༩',
        'Struct : {name: "Tenzin", age: ༢༥}',
        'Accès champ : person.name, assignation : person.age = ༣༠'
      ],
      builtinsTitle: 'Built-ins (Dzongkha)',
      builtinsItems: [
        'ཚད་(x) → longueur',
        'སྣོན་(arr, v) → push',
        'ཕྱིར་ལེན་(arr) → pop',
        'རིགས་(x) → type',
        'མིང་ཐོ་(struct) → clés',
        'བེད་སྤྱོད་(struct) → valeurs',
        'ཡོད་(container, v) → contient',
        'ནང་འཇུག་() → entrée'
      ],
      cliTitle: 'CLI',
      cliItems: [
        'Run : druk path/to/file.druk',
        'Compile : druk compile path/to/file.druk -o out.exe',
        'Execute bytecode : druk --execute file.bytecode'
      ],
      argsTitle: 'Arguments',
      argsBody: 'argv/argc disponibles comme globals + alias dzongkha.',
      argsItems: ['argv, argc', 'ནང་འཇུག་ཐོ་, ནང་འཇུག་གྲངས་'],
      wasmTitle: 'Architecture Web/WASM',
      wasmBody: 'Source (Monaco) → compilateur C++ en WASM → Bytecode → VM WASM → UI.'
    },
    footer: 'Druk • UI inspirée Bhoutan • Framework • Multilingue'
  },
  dz: {
    langName: 'རྫོང་ཁ',
    nav: {
      playground: 'Playground',
      docs: 'ལག་དེབ',
      examples: 'དཔེ་དེབ',
      language: 'སྐད་ཡིག'
    },
    hero: {
      title: 'Druk Playground',
      subtitle: 'རྫོང་ཁ་གཞི་བཟུང་ • འབྲུག་རྒྱལ་ཁབ་ལས་ཕྱེ • Web སྒྲིག',
      run: 'Run (WASM)',
      reset: 'Reset'
    },
    panels: {
      source: 'Source (Dzongkha)',
      status: 'WASM གནས་སྟངས',
      output: 'Output',
      outputHint: 'བཀོལ་སྤྱོད་བྱས་ན་བརྡ་འབུར་མཐོང་ཡོང་།'
    },
    docs: {
      enTitle: 'Technical Documentation (English)',
      dzTitle: 'ལག་དེབ་ཚན་པ། (Dzongkha)',
      overviewTitle: 'སྐད་ཡིག་གནས་བཟུང་།',
      overviewBody: 'Druk ནི་ Dzongkha གཞི་བཟུང་གི་སྐད་ཡིག་དང་བྱ་རིམ་ VM ཡིན། Lexer → Parser (AST) → Semantic → Codegen → VM.',
      runtimeTitle: 'VM ལམ་ལུགས།',
      runtimeItems: [
        'Stack-based bytecode VM དང call frame སྤྱོད་པ།',
        'Value: གྲངས་ཀ་, བདེན/རྫུན, ཡིག་རྟགས, array, struct, nil.',
        'Array = vector, Struct = key/value map.'
      ],
      collectionsTitle: 'Collections',
      collectionsItems: [
        'Array: [༡, ༢, ༣]',
        'Index: arr[༠], assignment: arr[༠] = ༩',
        'Struct: {name: "Tenzin", age: ༢༥}',
        'Field: person.name, assignment: person.age = ༣༠'
      ],
      builtinsTitle: 'Built-ins (Dzongkha)',
      builtinsItems: [
        'ཚད་(x) → རིང་ཚད',
        'སྣོན་(arr, v) → སྣོན་པ',
        'ཕྱིར་ལེན་(arr) → ཕྱིར་ལེན་པ',
        'རིགས་(x) → རིགས་དབྱེ',
        'མིང་ཐོ་(struct) → keys',
        'བེད་སྤྱོད་(struct) → values',
        'ཡོད་(container, v) → contains',
        'ནང་འཇུག་() → input'
      ],
      cliTitle: 'CLI',
      cliItems: [
        'Run: druk path/to/file.druk',
        'Compile: druk compile path/to/file.druk -o out.exe',
        'Execute bytecode: druk --execute file.bytecode'
      ],
      argsTitle: 'Arguments',
      argsBody: 'argv/argc གྲངས་ཐོ་ globals ནང་ཡོད།',
      argsItems: ['argv, argc', 'ནང་འཇུག་ཐོ་, ནང་འཇུག་གྲངས་'],
      wasmTitle: 'Web/WASM Architecture',
      wasmBody: 'Source (Monaco) → C++ compiler WASM → Bytecode → VM WASM → Output UI.'
    },
    footer: 'Druk • Bhutan-inspired UI • Framework • སྐད་གསུམ'
  }
};
