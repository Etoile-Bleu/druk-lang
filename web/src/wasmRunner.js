export async function loadWasm() {
  const module = await loadScript('/wasm/druk_wasm.js');
  const instance = await module();

  const run = (source) => {
    const ptr = instance.ccall('druk_wasm_run', 'number', ['string'], [source]);
    const out = instance.UTF8ToString(ptr);
    instance.ccall('druk_wasm_free', null, ['number'], [ptr]);
    return out;
  };

  return { run };
}

function loadScript(src) {
  return new Promise((resolve, reject) => {
    if (window.DrukWasm) {
      resolve(window.DrukWasm);
      return;
    }

    const script = document.createElement('script');
    script.src = src;
    script.async = true;
    script.onload = () => {
      if (window.DrukWasm) {
        resolve(window.DrukWasm);
      } else {
        reject(new Error('WASM module not found on window'));
      }
    };
    script.onerror = () => reject(new Error('Failed to load WASM JS'));
    document.head.appendChild(script);
  });
}
