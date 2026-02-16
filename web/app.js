const sourceEl = document.getElementById('source');
const outputEl = document.getElementById('output');
const runBtn = document.getElementById('runBtn');
const resetBtn = document.getElementById('resetBtn');

const defaultSource = `// Druk demo\nལས་ཀ་ རྩ_བ་() {\n    གྲངས་ཀ་ arr = [༡, ༢, ༣];\n    འབྲི་ "Hello Bhutan";\n    འབྲི་ ཚད་(arr);\n}\n\nརྩ_བ་();\n`;

sourceEl.value = defaultSource;

function mockRun(source) {
  const lines = source.split(/\r?\n/);
  const outputs = [];

  for (const line of lines) {
    const trimmed = line.trim();
    if (trimmed.startsWith('འབྲི་')) {
      const content = trimmed.replace('འབྲི་', '').trim();
      if (content.startsWith('"') && content.endsWith('"')) {
        outputs.push(content.slice(1, -1));
      } else {
        outputs.push(content);
      }
    }
  }

  if (outputs.length === 0) {
    return 'No output (Mock VM).';
  }
  return outputs.join('\n');
}

runBtn.addEventListener('click', () => {
  outputEl.textContent = mockRun(sourceEl.value);
});

resetBtn.addEventListener('click', () => {
  sourceEl.value = defaultSource;
  outputEl.textContent = '';
});
