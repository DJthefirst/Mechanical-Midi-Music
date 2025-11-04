# create-svelte

Everything you need to build a Svelte project, powered by [`create-svelte`](https://github.com/sveltejs/kit/tree/master/packages/create-svelte).

## Creating a project

If you're seeing this, you've probably already done this step. Congrats!

```bash
# create a new project in the current directory
npm create svelte@latest

# create a new project in my-app
npm create svelte@latest my-app
```

## Developing

Once you've created a project and installed dependencies with `npm install` (or `pnpm install` or `yarn`), start a development server:

```bash
npm run dev

# or start the server and open the app in a new browser tab
npm run dev -- --open
```

## Building

To create a production version of your app:

```bash
npm run build
```

You can preview the production build with `npm run preview`.

> To deploy your app, you may need to install an [adapter](https://kit.svelte.dev/docs/adapters) for your target environment.

## Node.js version

This project targets Node.js 22+. To ensure consistent developer environments, the repository includes a `.nvmrc` file set to `22` and the `package.json` declares the expected Node engine.

On Windows (PowerShell), install and use Node 22 via nvm-windows (or use the official installer):

Install nvm-windows: https://github.com/coreybutler/nvm-windows

Then run:

```powershell
# install Node 22
nvm install 22.0.0
# use Node 22
nvm use 22.0.0
# verify
node -v
```

If you prefer the official installer, download Node 22 from https://nodejs.org/ and run the installer.
