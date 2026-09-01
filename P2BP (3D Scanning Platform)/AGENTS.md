<!-- nx configuration start-->
<!-- Leave the start & end comments to automatically receive updates. -->

# General Guidelines for working with Nx

- For navigating/exploring the workspace, invoke the `nx-workspace` skill first - it has patterns for querying projects, targets, and dependencies
- When running tasks (for example build, lint, test, e2e, etc.), always prefer running the task through `nx` (i.e. `nx run`, `nx run-many`, `nx affected`) instead of using the underlying tooling directly
- Prefix nx commands with the workspace's package manager (e.g., `pnpm nx build`, `npm exec nx test`) - avoids using globally installed CLI
- You have access to the Nx MCP server and its tools, use them to help the user
- For Nx plugin best practices, check `node_modules/@nx/<plugin>/PLUGIN.md`. Not all plugins have this file - proceed without it if unavailable.
- NEVER guess CLI flags - always check nx_docs or `--help` first when unsure

## Scaffolding & Generators

- For scaffolding tasks (creating apps, libs, project structure, setup), ALWAYS invoke the `nx-generate` skill FIRST before exploring or calling MCP tools

## When to use nx_docs

- USE for: advanced config options, unfamiliar flags, migration guides, plugin configuration, edge cases
- DON'T USE for: basic generator syntax (`nx g @nx/react:app`), standard commands, things you already know
- The `nx-generate` skill handles generator discovery internally - don't call nx_docs just to look up generator syntax

<!-- nx configuration end-->

# Setup

- A fresh checkout is set up with one command: `bash tools/setup/setup.sh` (or `mise run setup` once mise is working — the script trusts and installs the toolchain itself, so it does not depend on how the local mise treats an untrusted config). It installs the pinned toolchain, runs `bun install`, syncs the Python app with uv, installs the pinned SwiftLint/SwiftFormat with mise for the iOS app, and installs the pre-commit hooks. It is idempotent and skips whatever the machine cannot do. `bash tools/setup/setup.sh --check` reports status without changing anything. See [README.md](README.md) for the full setup documentation.
- Local `.env` files are opt-in (`--with-env`) and never committed, because an env manager such as Doppler usually supplies the same values and a stray `.env` silently fills any gap it leaves. When they are generated, the Worker's keys come from the `secrets.required` list in `apps/p2bp-cf-worker/wrangler.jsonc`, so adding a required secret there is enough — do not maintain a parallel key list.

# Package management

- Bun is the only Node package manager and runtime in this repo. There is one lockfile, `bun.lock` at the workspace root, covering `apps/*` and `packages/*` Bun workspaces; `apps/p2bp-cf-worker` no longer has its own lockfile or `node_modules`. Never reintroduce `package-lock.json`, and run `bun install` from the root — installing from inside an app resolves to the root workspace anyway.
- The `apps/*` workspace glob deliberately matches only directories containing a `package.json`, so the Python (`p2bp-postprocessing`, uv) and Swift (`scanner-consolidator`, Xcode) apps are unaffected. Those keep their own toolchains; only their Nx task invocations go through Bun.
- Prefix Nx commands with `bunx` (`bunx nx run <project>:<target>`). Nx has supported Bun as a package manager since 19.1, but `nx migrate` and some generators shell out to the detected package manager — if one misbehaves, the Bun code path is the first thing to suspect.
- Toolchain versions are pinned in `mise.toml` (node + bun) and installed in CI via `jdx/mise-action@v4`. Bump versions there, not in workflow files; no workflow should pin `bun-version` or use `actions/setup-node` for Nx.
- Nx Cloud is disabled (`neverConnectToCloud` in `nx.json`), so the Nx guidance about custom Nx Agents launch templates for Bun does not apply here.
