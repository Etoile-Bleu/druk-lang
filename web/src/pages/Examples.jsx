import { useNavigate } from 'react-router-dom';
import { examples } from '../data/examples.js';
import { useI18n } from '../i18n/useI18n.js';

const STORAGE_EXAMPLE_KEY = 'druk_example';

export default function Examples() {
  const { t } = useI18n();
  const navigate = useNavigate();

  const onUse = (exampleId) => {
    window.localStorage.setItem(STORAGE_EXAMPLE_KEY, exampleId);
    navigate('/');
  };

  return (
    <section className="examples">
      <header className="examples-header">
        <h2>{t.nav.examples}</h2>
        <p>Pick a template and load it in the playground.</p>
      </header>
      <div className="examples-grid">
        {examples.map((item) => (
          <article key={item.id} className="example-card">
            <h3>{item.label}</h3>
            <pre>{item.code}</pre>
            <button className="btn primary" onClick={() => onUse(item.id)}>
              Use this example
            </button>
          </article>
        ))}
      </div>
    </section>
  );
}
