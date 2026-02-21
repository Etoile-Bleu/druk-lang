import { useEffect, useMemo, useState } from 'react';
import { useNavigate } from 'react-router-dom';
import { runAot } from '../aotRunner.js';
import { useI18n } from '../i18n/useI18n.js';
import { examples } from '../data/examples.js';

const STORAGE_EXAMPLE_KEY = 'druk_example';

function getExampleById(id) {
  return examples.find((item) => item.id === id) || examples[0];
}

export default function Playground() {
  const { t } = useI18n();
  const navigate = useNavigate();
  const [source, setSource] = useState(examples[0].code);
  const [output, setOutput] = useState('');
  const [status, setStatus] = useState('AOT Ready');
  const [activeExample, setActiveExample] = useState(examples[0]);
  const [isRunning, setIsRunning] = useState(false);

  useEffect(() => {
    const stored = window.localStorage.getItem(STORAGE_EXAMPLE_KEY);
    const example = getExampleById(stored || examples[0].id);
    setActiveExample(example);
    setSource(example.code);
  }, []);

  const onRun = async () => {
    if (isRunning) return;

    setIsRunning(true);
    setStatus('Running...');
    setOutput('');

    try {
      const result = await runAot(source);

      if (result.error) {
        setOutput(`Error: ${result.error}\n\n${result.stderr || ''}`);
        setStatus('Error');
      } else {
        const fullOutput = [];
        if (result.stdout) fullOutput.push(result.stdout);
        if (result.stderr) fullOutput.push(`Error Output:\n${result.stderr}`);

        setOutput(fullOutput.join('\n') || 'Program finished with no output.');
        setStatus('AOT Ready');
      }
    } catch (err) {
      setOutput(`Communication error: ${err.message}`);
      setStatus('Offline');
    } finally {
      setIsRunning(false);
    }
  };

  const onReset = () => {
    setSource(activeExample.code);
    setOutput('');
    setStatus('AOT Ready');
  };

  const onOpenExamples = () => navigate('/examples');

  const outputHint = useMemo(() => output || t.panels.outputHint, [output, t]);

  return (
    <main id="playground" className="layout">
      <section className="panel editor">
        <div className="panel-header">
          <span>{t.panels.source}</span>
          <span className="pill">{status}</span>
        </div>
        <div className="toolbar">
          <span className="toolbar-label">Example: {activeExample.label}</span>
          <button className="btn ghost" onClick={onOpenExamples}>See examples</button>
        </div>
        <textarea
          spellCheck={false}
          value={source}
          onChange={(event) => setSource(event.target.value)}
        />
        <div className="panel-actions">
          <button className="btn primary" onClick={onRun} disabled={isRunning}>
            {isRunning ? 'Running...' : t.hero.run}
          </button>
          <button className="btn ghost" onClick={onReset} disabled={isRunning}>
            {t.hero.reset}
          </button>
        </div>
      </section>

      <section className="panel output">
        <div className="panel-header">
          <span>{t.panels.output}</span>
          <span className="pill">AOT</span>
        </div>
        <pre className={status === 'Error' ? 'error-text' : ''}>{outputHint}</pre>
      </section>
    </main>
  );
}
