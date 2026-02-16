import { useEffect, useMemo, useState } from 'react';
import { translations } from './translations.js';
import { getStoredLanguage, setStoredLanguage } from './storage.js';

const supported = ['en', 'fr', 'dz'];

export function useI18n() {
  const [lang, setLang] = useState(getStoredLanguage());

  useEffect(() => {
    if (!supported.includes(lang)) {
      setLang('en');
    }
  }, [lang]);

  useEffect(() => {
    setStoredLanguage(lang);
  }, [lang]);

  const t = useMemo(() => translations[lang] || translations.en, [lang]);

  return {
    lang,
    setLang,
    t,
    supported
  };
}
