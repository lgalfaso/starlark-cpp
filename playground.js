const MONACO_VERSION = "0.57.0";
const MONACO_BASE = `https://cdn.jsdelivr.net/npm/monaco-editor@${MONACO_VERSION}/min`;

const DEFAULT_MODULES = {
  main: `load("lib", "greet")

def main():
    print(greet("Starlark"))
    print(greet("café"))
    print(greet("世界"))

main()
`,
  lib: `def greet(name):
    return "Hello, " + name + " - ☕"
`,
};

const outputEl = document.getElementById("output");
const errorsEl = document.getElementById("errors");
const statusEl = document.getElementById("status");
const runBtn = document.getElementById("run");
const moduleListEl = document.getElementById("module-list");
const addModuleBtn = document.getElementById("add-module");
const entrySelectEl = document.getElementById("entry-module");
const activeModuleLabelEl = document.getElementById("active-module-label");
const autoRunCheckboxEl = document.getElementById("auto-run");
const editorHostEl = document.getElementById("editor");

/** @type {EmscriptenModule | null} */
let wasmModule = null;
/** @type {import("monaco-editor").editor.IStandaloneCodeEditor | null} */
let editor = null;

/** @type {Map<string, string>} */
const moduleSources = new Map(Object.entries(DEFAULT_MODULES));
let activeModule = "main";
let entryModule = "main";
let playgroundReady = false;

const STATUS_READY = "Ready.";
const AUTO_RUN_DELAY_MS = 3000;

let autoRunTimerId = null;

function encodeUtf8(text) {
  const byteLength = wasmModule.lengthBytesUTF8(text) + 1;
  const pointer = wasmModule._malloc(byteLength);
  wasmModule.stringToUTF8(text, pointer, byteLength);
  return pointer;
}

function setStatus(message) {
  statusEl.textContent = message;
}

function setReadyStatus() {
  setStatus(STATUS_READY);
}

function cancelAutoRun() {
  if (autoRunTimerId !== null) {
    clearTimeout(autoRunTimerId);
    autoRunTimerId = null;
  }
}

function isAutoRunEnabled() {
  return autoRunCheckboxEl.checked;
}

function scheduleAutoRun() {
  if (!playgroundReady || !isAutoRunEnabled()) {
    return;
  }
  cancelAutoRun();
  autoRunTimerId = setTimeout(() => {
    autoRunTimerId = null;
    void runProgram();
  }, AUTO_RUN_DELAY_MS);
}

function clearPanes() {
  outputEl.textContent = "";
  errorsEl.textContent = "";
}

function monacoTheme() {
  return window.matchMedia("(prefers-color-scheme: dark)").matches ? "vs-dark" : "vs";
}

function syncEditorToModule() {
  if (!editor) {
    return;
  }
  moduleSources.set(activeModule, editor.getValue());
}

function loadModuleIntoEditor(name) {
  if (!editor) {
    return;
  }
  const source = moduleSources.get(name) ?? "";
  editor.setValue(source);
  activeModuleLabelEl.textContent = name;
}

function isValidModuleName(name) {
  if (!name || name.includes("/") || name.includes("\\")) {
    return false;
  }
  // UTF-8 module names are fine; disallow ASCII control characters only.
  return !/[\x00-\x1f\x7f]/.test(name);
}

function nextModuleName() {
  let index = 1;
  while (moduleSources.has(`module${index}`)) {
    index += 1;
  }
  return `module${index}`;
}

function selectModule(name) {
  if (activeModule === name) {
    updateModuleListActiveState();
    return;
  }
  syncEditorToModule();
  activeModule = name;
  loadModuleIntoEditor(name);
  updateModuleListActiveState();
}

function updateModuleListActiveState() {
  for (const item of moduleListEl.querySelectorAll(".module-item")) {
    item.classList.toggle("active", item.dataset.moduleKey === activeModule);
  }
}

function setModuleNameError(inputEl, message) {
  inputEl.classList.add("invalid");
  inputEl.setAttribute("aria-invalid", "true");
  inputEl.title = message;
}

function clearModuleNameError(inputEl) {
  inputEl.classList.remove("invalid");
  inputEl.removeAttribute("aria-invalid");
  inputEl.title = "Module name (editable)";
}

function commitModuleRename(previousName, inputEl) {
  const next = inputEl.value.trim();
  if (next === previousName) {
    inputEl.value = previousName;
    clearModuleNameError(inputEl);
    return true;
  }
  if (!next || !isValidModuleName(next)) {
    inputEl.value = previousName;
    setModuleNameError(inputEl, "Invalid name: non-empty, no slashes, no control characters.");
    return false;
  }
  if (moduleSources.has(next)) {
    inputEl.value = previousName;
    setModuleNameError(inputEl, "A module with that name already exists.");
    return false;
  }

  syncEditorToModule();
  const source = moduleSources.get(previousName) ?? "";
  moduleSources.delete(previousName);
  moduleSources.set(next, source);
  if (activeModule === previousName) {
    activeModule = next;
  }
  if (entryModule === previousName) {
    entryModule = next;
  }
  clearModuleNameError(inputEl);
  renderEntrySelect();
  renderModuleList();
  scheduleAutoRun();
  return true;
}

function renderModuleList(focusModule = null) {
  moduleListEl.replaceChildren();
  const names = [...moduleSources.keys()].sort((a, b) => a.localeCompare(b));

  for (const name of names) {
    const item = document.createElement("li");
    item.className = "module-item" + (name === activeModule ? " active" : "");
    item.dataset.moduleKey = name;

    const nameRow = document.createElement("div");
    nameRow.className = "module-name-row";

    const nameInput = document.createElement("input");
    nameInput.type = "text";
    nameInput.className = "module-name-input";
    nameInput.value = name;
    nameInput.spellcheck = false;
    nameInput.autocomplete = "off";
    nameInput.title = "Module name (editable)";
    nameInput.setAttribute("aria-label", `Module name ${name}`);
    nameInput.addEventListener("focus", () => {
      selectModule(name);
    });
    nameInput.addEventListener("keydown", (event) => {
      if (event.key === "Enter") {
        event.preventDefault();
        nameInput.blur();
      } else if (event.key === "Escape") {
        event.preventDefault();
        nameInput.value = name;
        clearModuleNameError(nameInput);
        nameInput.blur();
      }
    });
    nameInput.addEventListener("blur", () => {
      commitModuleRename(name, nameInput);
    });

    nameRow.append(nameInput);
    if (name === entryModule) {
      const badge = document.createElement("span");
      badge.className = "entry-badge";
      badge.textContent = "E";
      badge.title = "Entry module";
      nameRow.append(badge);
    }

    const deleteBtn = document.createElement("button");
    deleteBtn.type = "button";
    deleteBtn.className = "module-action-btn icon-btn danger";
    deleteBtn.title = "Remove module";
    deleteBtn.textContent = "×";
    deleteBtn.disabled = moduleSources.size <= 1;
    deleteBtn.addEventListener("click", () => {
      if (moduleSources.size <= 1) {
        return;
      }
      moduleSources.delete(name);
      if (activeModule === name) {
        activeModule = moduleSources.keys().next().value;
        loadModuleIntoEditor(activeModule);
      }
      if (entryModule === name) {
        entryModule = activeModule;
      }
      renderEntrySelect();
      renderModuleList();
      scheduleAutoRun();
    });

    item.append(nameRow, deleteBtn);
    moduleListEl.append(item);

    if (focusModule === name) {
      nameInput.focus();
      nameInput.select();
    }
  }
}

function renderEntrySelect() {
  const previous = entryModule;
  entrySelectEl.replaceChildren();
  for (const name of [...moduleSources.keys()].sort()) {
    const option = document.createElement("option");
    option.value = name;
    option.textContent = name;
    entrySelectEl.append(option);
  }
  if (moduleSources.has(previous)) {
    entrySelectEl.value = previous;
    entryModule = previous;
  } else {
    entryModule = entrySelectEl.value;
  }
}

function configureMonaco() {
  return new Promise((resolve, reject) => {
    if (!window.require) {
      reject(new Error("Monaco loader failed to load"));
      return;
    }
    window.require.config({ paths: { vs: `${MONACO_BASE}/vs` } });
    window.require(
      ["vs/editor/editor.main"],
      () => {
        editor = monaco.editor.create(editorHostEl, {
          value: moduleSources.get(activeModule) ?? "",
          language: "python",
          theme: monacoTheme(),
          automaticLayout: true,
          minimap: { enabled: false },
          fontSize: 14,
          scrollBeyondLastLine: false,
        });
        window.matchMedia("(prefers-color-scheme: dark)").addEventListener("change", () => {
          monaco.editor.setTheme(monacoTheme());
        });
        editor.onDidChangeModelContent(() => {
          scheduleAutoRun();
        });
        new ResizeObserver(() => {
          editor.layout();
        }).observe(editorHostEl);
        resolve();
      },
      reject,
    );
  });
}

async function loadWasm() {
  if (typeof createStarlarkModule !== "function") {
    throw new Error("createStarlarkModule is not defined (starlark_wasm.js missing?)");
  }
  wasmModule = await createStarlarkModule({
    locateFile: (path) => path,
  });
}

async function runProgram() {
  if (!wasmModule || !editor) {
    return;
  }
  cancelAutoRun();
  clearPanes();
  setStatus("Running…");
  runBtn.disabled = true;

  try {
    syncEditorToModule();
    entryModule = entrySelectEl.value;

    wasmModule._starlark_clear_modules();
    for (const [name, source] of moduleSources.entries()) {
      const namePtr = encodeUtf8(name);
      const sourcePtr = encodeUtf8(source);
      wasmModule._starlark_define_module(namePtr, sourcePtr);
      wasmModule._free(namePtr);
      wasmModule._free(sourcePtr);
    }

    const entryPtr = encodeUtf8(entryModule);
    const resultPtr = wasmModule._starlark_run(entryPtr);
    wasmModule._free(entryPtr);

    if (resultPtr === 0) {
      errorsEl.textContent = "starlark_run returned null";
      return;
    }

    const jsonText = wasmModule.UTF8ToString(resultPtr);
    wasmModule._free(resultPtr);

    const result = JSON.parse(jsonText);
    if (result.ok) {
      outputEl.textContent = result.output ?? "";
    } else {
      const errors = Array.isArray(result.errors) ? result.errors : [];
      errorsEl.textContent = errors.join("\n");
    }
  } catch (err) {
    errorsEl.textContent = String(err);
  } finally {
    runBtn.disabled = false;
    if (playgroundReady) {
      setReadyStatus();
    }
  }
}

addModuleBtn.addEventListener("click", () => {
  const name = nextModuleName();
  moduleSources.set(name, "# new module\n");
  activeModule = name;
  loadModuleIntoEditor(name);
  renderEntrySelect();
  renderModuleList(name);
  scheduleAutoRun();
});

entrySelectEl.addEventListener("change", () => {
  entryModule = entrySelectEl.value;
  renderModuleList();
  scheduleAutoRun();
});

runBtn.addEventListener("click", () => {
  cancelAutoRun();
  void runProgram();
});

autoRunCheckboxEl.addEventListener("change", () => {
  if (isAutoRunEnabled()) {
    scheduleAutoRun();
  } else {
    cancelAutoRun();
  }
});

async function boot() {
  try {
    renderEntrySelect();
    renderModuleList();
    await Promise.all([configureMonaco(), loadWasm()]);
    loadModuleIntoEditor(activeModule);
    runBtn.disabled = false;
    playgroundReady = true;
    setReadyStatus();
    scheduleAutoRun();
  } catch (err) {
    errorsEl.textContent = String(err);
    setStatus("Failed to initialize playground.");
  }
}

void boot();
