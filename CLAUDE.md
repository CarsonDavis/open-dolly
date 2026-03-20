# Indexing Protocol

You are responsible for maintaining the repo's `INDEX.md` — the map of the codebase. If the index is wrong, you will make wrong decisions about what to read.

## Start

Read `INDEX.md` before doing anything else. Do not rely on memory or assumptions from previous sessions.

## Entry Format

Every file or cohesive group of files gets one table row:

| Column | What to write |
|--------|---------------|
| **Path** | Relative path to file or directory |
| **Purpose** | One sentence: what it is and why it exists |
| **Key Details** | 2–5 facts needed to decide read vs. skip: data formats, interfaces, config keys, status, dependencies, interactions, etc |

Example:

```
| Path | Purpose | Key Details |
|------|---------|-------------|
| `extract.py` | Scrapes Signal messages via ADB/uiautomator | Outputs `extraction_state_clean.json`; requires Android device connected |
| `date_extraction/` | OCR pipeline for date headers invisible to uiautomator | Uses Tesseract PSM 7; outputs `messages_with_dates.json` |
| `group_books.py` | Groups messages by book using hybrid rules + LLM | Reads `read_books.csv` (Goodreads); outputs `book_messages.json` |
```

Rules:
- If you can't decide read/skip from an entry alone, the entry is missing key details. Fix it.
- **Index** docs like `README.md` — track what's documented where.
- **Don't index** boilerplate (`.gitignore`, `package.json`, lock files), generated output (`dist/`, `build/`, `.venv/`), or individual files inside a directory already indexed as a group.
- For repos with distinct subsystems, add a **Reading Guide** table mapping task types → required reading:

```
| Task | Read First |
|------|------------|
| Changing data models | `docs/data-models.md`, `models/book.py` |
| API endpoints | `docs/api-reference.md`, `routes/` |
| Infrastructure | `docs/architecture.md`, `cdk/` |
| Frontend | `dashboard/README.md` |
```

## Sub-Indexes

When a directory is too large for the root index, give it its own `INDEX.md`. The root references the directory; the sub-index covers everything inside. Chain as deep as needed: root → `frontend/INDEX.md` → `frontend/components/INDEX.md`.

```
## Root INDEX.md
| Path | Purpose | Key Details |
|------|---------|-------------|
| `frontend/` | React web app | Has its own `INDEX.md`; Next.js 14, USWDS components |

## frontend/INDEX.md
| Path | Purpose | Key Details |
|------|---------|-------------|
| `components/` | Shared UI components | Has its own `INDEX.md`; all use USWDS design tokens |
| `pages/api/` | API routes | REST endpoints; auth via middleware |
```

Rules:
- A sub-index **owns** its directory — the parent must not duplicate its entries.
- Sub-indexes use the same format (Path, Purpose, Key Details).
- Update the sub-index when files inside change — not the root.
- When you create or remove a sub-index, update the parent.

## After Every Change

Documentation is part of the deliverable, not a follow-up task. Before moving to the next task:

1. **Update INDEX.md** — add new files, remove deleted ones, revise changed entries.
2. **Fix linked docs** — if your work changes what a referenced doc says, fix it now.
3. **Grep for staleness** — search the repo for references to anything renamed, moved, or removed.
4. **Update, don't duplicate** — before creating a new doc, check the index for an existing one. Revise it instead.
5. **Delete, don't orphan** — when you remove a file, remove its index entry and all references.
6. **Use status when useful** — for planning docs or specs, mark `draft` / `active` / `deprecated`.
