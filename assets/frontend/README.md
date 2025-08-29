# Local Web
This directory contains the source code for the web interface of BUSY Bar.

## Setup
- Node v20.17.1 or later is required.
- pnpm v9.12.3 or later is required.

## Development
Install dependencies with `pnpm install` in the `assets/frontend` directory. Use `pnpm dev` to start a development server.

1. Commit the files you've changed. If the dev server has wiped out the previous build files, do not include these deletions in the commit.
2. Rebuild the frontend with `pnpm build`.
3. Commit the new build files with the commit message `frontend: build`.