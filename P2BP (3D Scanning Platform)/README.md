# P2BP Monorepo

An [Nx](https://nx.dev) workspace holding the three P2BP apps and the tooling they share.

| Project                     | What it is                                                              | Stack            |
| --------------------------- | ----------------------------------------------------------------------- | ---------------- |
| `apps/p2bp-cf-worker`       | Hono API on Cloudflare Workers — Better Auth, Drizzle, D1, R2, Queues   | Bun + TypeScript |
| `apps/p2bp-postprocessing`  | EC2 queue worker that merges LiDAR zone scans into project point clouds | Python 3.12 + uv |
| `apps/scanner-consolidator` | SwiftUI iOS app for drawing project boundaries and capturing scans      | Swift + Xcode    |

Each app keeps its own deep documentation; this file covers getting the workspace running.

## Setup

### 1. Install mise

[mise](https://mise.jdx.dev) is the only prerequisite. It installs the Node and Bun versions pinned in [mise.toml](mise.toml), so nobody has to match versions by hand.

```bash
brew install mise
```

On Linux, or without Homebrew, use `curl https://mise.run | sh`. Then activate it in your shell so `bun` and `node` resolve to the pinned versions:

```bash
echo 'eval "$(mise activate zsh)"' >> ~/.zshrc && exec zsh
```

### 2. Run setup

```bash
bash tools/setup/setup.sh
```

That is the whole setup. It is idempotent — re-run it after any pull that changes dependencies, and it will only do the work that is still missing.

The script trusts and installs the pinned toolchain itself, so it works on a fresh clone whatever version of mise you have. Once mise is set up, `mise run setup` and `bun run setup` are equivalent shorthands.

It does the following, skipping anything your machine cannot do yet and telling you how to fill the gap:

| Step      | Action                                                                            |
| --------- | --------------------------------------------------------------------------------- |
| Toolchain | `mise install` — Node and Bun at the pinned versions                              |
| Workspace | `bun install --frozen-lockfile` at the root, covering every Bun workspace         |
| Python    | `uv sync --locked` for `apps/p2bp-postprocessing` (needs `uv`)                    |
| iOS       | `mise install` plus a SwiftLint/SwiftFormat version check (needs macOS and Xcode) |
| Git hooks | `pre-commit install`, which installs both the `pre-commit` and `pre-push` hooks   |

Useful flags:

```bash
bash tools/setup/setup.sh --check                     # report status, change nothing
bash tools/setup/setup.sh --skip-python --skip-ios    # Worker only
bash tools/setup/setup.sh --with-env                  # also write starter .env files
```

`--skip-hooks` exists as well; `--help` lists everything.

### 3. Supply local secrets

Each app needs credentials before it will run locally:

| App                        | Needed for                                      | Values come from                                  |
| -------------------------- | ----------------------------------------------- | ------------------------------------------------- |
| `apps/p2bp-cf-worker`      | `wrangler dev`, Drizzle migrations, Better Auth | Cloudflare dashboard, Google/Apple OAuth, R2, AWS |
| `apps/p2bp-postprocessing` | Cloudflare Queue pulls and R2 transfers         | Cloudflare dashboard (queue **id**, not its name) |

Supply them however you prefer — Doppler or another env manager, or local `.env` files. If you want the files, setup can generate them:

```bash
bash tools/setup/setup.sh --with-env
```

That writes `apps/p2bp-cf-worker/.env` and `apps/p2bp-postprocessing/.env` with the right keys and empty values, and never overwrites an existing file. The Worker's keys are read out of [`wrangler.jsonc`](apps/p2bp-cf-worker/wrangler.jsonc), so they always match the secrets the Worker declares as required, and `BETTER_AUTH_SECRET` is generated for you.

Generation is opt-in on purpose. Every consumer here — Bun, `dotenv`, `python-dotenv`, and Wrangler — lets an injected environment variable win over the same key in a `.env` file, so a local file never overrides `doppler run -- …`. What it does do is quietly fill any gap your env manager leaves, which turns a missing credential into an empty value instead of an error.

Both files are git-ignored. [`.env.example`](.env.example) at the root is the reference list of every variable used anywhere in the repo. Deployed environments read their values from Cloudflare Worker secrets, GitHub Actions secrets, or the EC2 `EnvironmentFile` — never from these files.

For the Worker, also authenticate Wrangler once:

```bash
bunx wrangler login
```

### Optional prerequisites

Setup skips these cleanly if they are missing, so install only what you need:

| Tool         | Needed for              | Install                                        |
| ------------ | ----------------------- | ---------------------------------------------- |
| `uv`         | the Python queue worker | `brew install uv`                              |
| Xcode        | the iOS app             | App Store (`mise` supplies the lint tools)     |
| `pre-commit` | the git hooks           | installed automatically when `uv` is available |

### 4. Check it worked

```bash
bash tools/setup/setup.sh --check
```

This one reports and changes nothing, which is why it is not a mise task: `mise run` installs any missing pinned tool before the task starts.

Then run something end to end. For the Worker:

```bash
bunx nx run p2bp-cf-worker:serve
```

Its API is at `http://localhost:8787/api`, with OpenAPI docs at `/api/open-api` and Scalar at `/api/scalar`. On a fresh checkout, apply the local D1 migrations first:

```bash
cd apps/p2bp-cf-worker && bun run db:migrate:local:dev
```

## Everyday commands

Run tasks through Nx rather than the underlying tools, so caching and dependencies are respected:

```bash
bunx nx run <project>:<target>
```

| Project                | Targets                                                                                    |
| ---------------------- | ------------------------------------------------------------------------------------------ |
| `p2bp-cf-worker`       | `serve`, `test`, `lint`, `typecheck`, `check-wrangler-config`, `check-generated-files`     |
| `p2bp-postprocessing`  | `run`, `test`, `lint`, `sync`                                                              |
| `scanner-consolidator` | `build`, `test`, `lint`, `format`, `analyze`, `build-for-testing`, `test-without-building` |

Across projects:

```bash
bunx nx run-many -t test          # every project
bunx nx affected -t lint test     # only what your branch touched
bunx nx graph                     # explore the workspace
bunx nx sync                      # refresh TypeScript project references
bunx nx format:check --base=origin/main
```

## Git hooks

[`.pre-commit-config.yaml`](.pre-commit-config.yaml) installs both hook types:

- **commit** — whitespace and YAML checks, `actionlint`, `shellcheck`, Biome on the Worker, Ruff on the Python app, SwiftFormat/SwiftLint and tooling checks on the iOS app.
- **push** — Worker tests, typecheck, Wrangler config policy, and generated-file checks; SwiftLint analyzer and Xcode tests.

Run them on demand with `pre-commit run --all-files`, skip one with `SKIP=<hook-id> git commit`, and bypass everything with `git commit --no-verify` when you genuinely need to.

## Deeper documentation

- Cloudflare Worker: [README](apps/p2bp-cf-worker/README.md) and [DEVELOPMENT](apps/p2bp-cf-worker/DEVELOPMENT.md) — deploys, D1 migrations, Better Auth, Cloudflare object policy
- Queue worker: [README](apps/p2bp-postprocessing/README.md) — queue setup, registration tuning, systemd services
- iOS app: [DEVELOPMENT](apps/scanner-consolidator/DEVELOPMENT.md) — xcconfig layering, simulator selection, CI split
- Workspace conventions for humans and agents: [AGENTS.md](AGENTS.md)

## Workspace notes

- Bun is the only Node package manager here. One lockfile, [`bun.lock`](bun.lock), covers `apps/*` and `packages/*`; always install from the root and never reintroduce `package-lock.json`.
- Toolchain versions live in [`mise.toml`](mise.toml) and are installed in CI by `jdx/mise-action@v4`. Bump them there, not in workflow files.
- Nx Cloud is disabled (`neverConnectToCloud` in [`nx.json`](nx.json)); caching is local only.
- Generate a new library with `bunx nx g @nx/js:lib packages/pkg1 --publishable --importPath=@my-org/pkg1`, and release publishable packages with `bunx nx release` (add `--dry-run` first).

## Troubleshooting

**`bun: command not found` after setup** — mise is installed but not activated in your shell. Add the `mise activate` line from step 1, or prefix commands with `mise exec --`.

**`mise run setup` says the config is untrusted** — older mise versions ask for trust before they will read `mise.toml` to find the task, so the trust step inside the script never gets to run. Current mise loads this config without prompting, since it only contains plain tool versions and tasks. Either update mise, run `mise trust` once in the repo root, or use `bash tools/setup/setup.sh`, which handles it.

**`pre-commit` refuses to install: "Cowardly refusing to install hooks with core.hooksPath set"** — something already points git at a custom hooks path, and pre-commit will not install beside it. Find out what set it before changing anything, since whatever did may depend on it:

```bash
git config --show-origin --get-all core.hooksPath
```

Once that is resolved in whichever way suits your setup, run `pre-commit install`.

**`wrangler dev` warns about missing secrets** — nothing is supplying that key: either your env manager is not providing it, or it is present but empty in a local `.env`. `bunx nx run p2bp-cf-worker:check-wrangler-config` lists what the Worker requires.

**Python deps look stale** — `bunx nx run p2bp-postprocessing:sync` re-syncs from `uv.lock`. Setup runs this too.

**Xcode picks the wrong simulator** — set `IOS_SIMULATOR_ID`, or `IOS_SIMULATOR_NAME` plus `IOS_SIMULATOR_OS`, before running the `scanner-consolidator` targets.
