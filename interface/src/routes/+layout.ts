import type { LayoutLoad } from './$types';

// This can be false if you're using a fallback (i.e. SPA mode)
export const prerender = false;
export const ssr = false;

export const load = (async ({ fetch }) => {
	const result = await fetch('/rest/features');
	const item = await result.json();
	return {
		features: item,
		title: 'EJ2640 Platform - Fucking Machine',
		github: 'doctor-daddy/LUST-motion',
		copyright: '2018 Doctor Daddy',
		appName: 'EJ2640 Platform - Fucking Machine'
	};
}) satisfies LayoutLoad;
