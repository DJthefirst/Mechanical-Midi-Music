import { vitePreprocess } from '@sveltejs/kit/vite';
import adapter from '@sveltejs/adapter-static';

/** @type {import('@sveltejs/kit').Config} */
const config = {
	// Consult https://kit.svelte.dev/docs/integrations#preprocessors
	// for more information about preprocessors
	preprocess: [vitePreprocess({})],

	//Mute A11y Accessibility warnings
	onwarn: (warning, handler) => {
		if (warning.code.startsWith('a11y-')) {
			return;
		}
		handler(warning);
	},

	kit: {
        adapter: adapter({
            //paths: { base: "/PERSONAL_PATH" },
			pages: 'build',
            assets: 'build',
            fallback: 'index.html',
            precompress: false,
			strict: true,
        })
	}
};

export default config;
