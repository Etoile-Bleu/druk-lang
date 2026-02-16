export const examples = [
  {
    id: 'hello',
    label: 'Hello Bhutan',
    code: `// Druk demo
ལས་ཀ་ རྩ_བ་() {
    གྲངས་ཀ་ arr = [༡, ༢, ༣];
    འབྲི་ "Kuzuzangpo";
    འབྲི་ ཚད་(arr);
}

རྩ_བ་();
`
  },
  {
    id: 'arrays',
    label: 'Arrays',
    code: `// Arrays
ལས་ཀ་ རྩ_བ་() {
    གྲངས་ཀ་ arr = [༡, ༢, ༣, ༤, ༥];
    འབྲི་ arr[༠];
    འབྲི་ arr[༢];
    འབྲི་ arr[༤];
}

རྩ_བ་();
`
  },
  {
    id: 'structs',
    label: 'Structs',
    code: `// Structs
ལས་ཀ་ རྩ_བ་() {
    གྲངས་ཀ་ person = {name: "Tenzin", age: ༢༥};
    འབྲི་ person.name;
    འབྲི་ person.age;
}

རྩ_བ་();
`
  },
  {
    id: 'assignment',
    label: 'Assignment',
    code: `// Assignment
ལས་ཀ་ test() {
    གྲངས་ཀ་ arr = [༡, ༢, ༣];
    arr[༡] = ༩༩;
    འབྲི་ arr[༡];

    གྲངས་ཀ་ person = {name: "John", age: ༢༥};
    person.age = ༣༠;
    འབྲི་ person.age;
}

test();
`
  },
  {
    id: 'game',
    label: 'Game Logic',
    code: `// Tic-Tac-Toe (logic)
ལས་ཀ་ རྩ_བ་() {
    གྲངས་ཀ་ board = [༡, ༡, ༡, ༢, ༢, ༠, ༠, ༠, ༠];
    འབྲི་ "Winner:";
    འབྲི་ "X";
}

རྩ_བ་();
`
  }
];
