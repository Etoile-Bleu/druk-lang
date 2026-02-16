import { useI18n } from '../i18n/useI18n.js';

export default function Docs() {
  const { t } = useI18n();

  return (
    <section id="docs" className="docs">
      <div className="doc-card">
        <h2>{t.docs.enTitle}</h2>
        <h3>{t.docs.overviewTitle}</h3>
        <p>{t.docs.overviewBody}</p>
        <h3>{t.docs.runtimeTitle}</h3>
        <ul>
          {t.docs.runtimeItems.map((item) => (
            <li key={item}>{item}</li>
          ))}
        </ul>
        <h3>{t.docs.collectionsTitle}</h3>
        <ul>
          {t.docs.collectionsItems.map((item) => (
            <li key={item}>{item}</li>
          ))}
        </ul>
        <h3>{t.docs.builtinsTitle}</h3>
        <ul>
          {t.docs.builtinsItems.map((item) => (
            <li key={item}>{item}</li>
          ))}
        </ul>
        <h3>{t.docs.cliTitle}</h3>
        <ul>
          {t.docs.cliItems.map((item) => (
            <li key={item}>{item}</li>
          ))}
        </ul>
        <h3>{t.docs.argsTitle}</h3>
        <p>{t.docs.argsBody}</p>
        <ul>
          {t.docs.argsItems.map((item) => (
            <li key={item}>{item}</li>
          ))}
        </ul>
        <h3>{t.docs.wasmTitle}</h3>
        <p>{t.docs.wasmBody}</p>
      </div>

      <div className="doc-card">
        <h2>{t.docs.dzTitle}</h2>
        <h3>{t.docs.overviewTitle}</h3>
        <p>{t.docs.overviewBody}</p>
        <h3>{t.docs.runtimeTitle}</h3>
        <ul>
          {t.docs.runtimeItems.map((item) => (
            <li key={item}>{item}</li>
          ))}
        </ul>
        <h3>{t.docs.collectionsTitle}</h3>
        <ul>
          {t.docs.collectionsItems.map((item) => (
            <li key={item}>{item}</li>
          ))}
        </ul>
        <h3>{t.docs.builtinsTitle}</h3>
        <ul>
          {t.docs.builtinsItems.map((item) => (
            <li key={item}>{item}</li>
          ))}
        </ul>
        <h3>{t.docs.cliTitle}</h3>
        <ul>
          {t.docs.cliItems.map((item) => (
            <li key={item}>{item}</li>
          ))}
        </ul>
        <h3>{t.docs.argsTitle}</h3>
        <p>{t.docs.argsBody}</p>
        <ul>
          {t.docs.argsItems.map((item) => (
            <li key={item}>{item}</li>
          ))}
        </ul>
        <h3>{t.docs.wasmTitle}</h3>
        <p>{t.docs.wasmBody}</p>
      </div>
    </section>
  );
}
