import { BrowserRouter, Route, Routes } from 'react-router-dom';
import { useI18n } from './i18n/useI18n.js';
import Playground from './pages/Playground.jsx';
import Docs from './pages/Docs.jsx';
import Examples from './pages/Examples.jsx';

export default function App() {
  return (
    <BrowserRouter>
      <Layout />
    </BrowserRouter>
  );
}

function Layout() {
  const { lang, setLang, t, supported } = useI18n();

  return (
    <div className="app">
      <div className="bg-pattern" />
      <nav className="navbar">
        <div className="nav-brand">
          <span className="nav-mark">འབྲུག</span>
          <span>Druk Web</span>
        </div>
        <div className="nav-links">
          <a href="/">{t.nav.playground}</a>
          <a href="/examples">{t.nav.examples}</a>
          <a href="/docs">{t.nav.docs}</a>
        </div>
        <div className="nav-actions">
          <label className="nav-label">
            {t.nav.language}
            <select value={lang} onChange={(event) => setLang(event.target.value)}>
              {supported.map((code) => (
                <option key={code} value={code}>
                  {translationsLabel(code)}
                </option>
              ))}
            </select>
          </label>
        </div>
      </nav>

      <header className="hero">
        <div className="brand">
          <div className="brand-mark">འབྲུག</div>
          <div className="brand-text">
            <h1>{t.hero.title}</h1>
            <p>{t.hero.subtitle}</p>
          </div>
        </div>
      </header>

      <Routes>
        <Route path="/" element={<Playground />} />
        <Route path="/docs" element={<Docs />} />
        <Route path="/examples" element={<Examples />} />
      </Routes>

      <footer className="footer">{t.footer}</footer>
    </div>
  );
}

function translationsLabel(code) {
  switch (code) {
    case 'en':
      return 'English';
    case 'fr':
      return 'Français';
    case 'dz':
      return 'རྫོང་ཁ';
    default:
      return code;
  }
}
