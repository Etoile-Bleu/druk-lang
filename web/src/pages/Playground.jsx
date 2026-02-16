import { useEffect, useMemo, useState } from 'react';
import { useNavigate } from 'react-router-dom';
import { loadWasm } from '../wasmRunner.js';
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
  const [status, setStatus] = useState('Loading WASM...');
  const [runner, setRunner] = useState(null);
  const [activeExample, setActiveExample] = useState(examples[0]);

  useEffect(() => {
    const stored = window.localStorage.getItem(STORAGE_EXAMPLE_KEY);
    const example = getExampleById(stored || examples[0].id);
    setActiveExample(example);
    setSource(example.code);
  }, []);

  useEffect(() => {
    let cancelled = false;
    loadWasm()
      .then((api) => {
        if (cancelled) return;
        setRunner(api);
        setStatus('WASM ready');
      })
      .catch(() => {
        if (cancelled) return;
        setStatus('WASM missing (build required)');
      });
    return () => {
      cancelled = true;
    };
  }, []);

  const onRun = () => {
    if (!runner) {
      setOutput('WASM not loaded. Build the wasm bundle first.');
      return;
    }
    setOutput(runner.run(source));
  };

  const onReset = () => {
    setSource(activeExample.code);
    setOutput('');
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
          <button className="btn primary" onClick={onRun}>{t.hero.run}</button>
          <button className="btn ghost" onClick={onReset}>{t.hero.reset}</button>
        </div>
      </section>

      <section className="panel output">
        <div className="panel-header">
          <span>{t.panels.output}</span>
          <span className="pill">WASM</span>
        </div>
        <pre>{outputHint}</pre>
      </section>
    </main>
  );
}
