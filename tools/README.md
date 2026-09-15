# tools/

Offline development/import/packaging utilities.

Nothing in this directory should require Roblox online services at runtime.


## Early map importer

The tools directory should contain a standalone importer that can inspect locally available Natural Disaster Survival source files, import supported place/model data, and emit the project's internal map package.

The importer should be useful before the engine is feature-complete. Prefer a staged import with diagnostics over an all-or-nothing conversion.

Suggested commands:

```text
nds-import scan <source>
nds-import inspect <source>
nds-import map <source> --out <package>
nds-import validate <package>
nds-import report <package>
```

Do not automatically copy source files into distributable release bundles.
