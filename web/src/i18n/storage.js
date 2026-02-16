const STORAGE_KEY = 'druk_lang';
const COOKIE_KEY = 'druk_lang';

export function getStoredLanguage() {
  if (typeof window === 'undefined') return 'en';
  const stored = window.localStorage.getItem(STORAGE_KEY);
  if (stored) return stored;
  const fromCookie = readCookie(COOKIE_KEY);
  return fromCookie || 'en';
}

export function setStoredLanguage(lang) {
  if (typeof window === 'undefined') return;
  window.localStorage.setItem(STORAGE_KEY, lang);
  writeCookie(COOKIE_KEY, lang, 365);
}

function readCookie(name) {
  const value = `; ${document.cookie}`;
  const parts = value.split(`; ${name}=`);
  if (parts.length === 2) return parts.pop().split(';').shift();
  return '';
}

function writeCookie(name, value, days) {
  const expires = new Date(Date.now() + days * 864e5).toUTCString();
  document.cookie = `${name}=${value}; expires=${expires}; path=/`;
}
