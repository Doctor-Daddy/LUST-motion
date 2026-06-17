import { writable } from 'svelte/store';
import type { Environment } from '$lib/types/models';

let environment_data: Environment = {
	depth: 225,
	max_rate: 600,
	max_velocity: 4064,
	heartbeat_mode: 0,
	patterns: <string[]>[],
	max_current: 5,
	max_voltage: 40,
	motor: 'VIRTUAL'
};

function createEnvironment() {
	const { subscribe, set, update } = writable(environment_data);

	function getHeartbeatMode() {
		return environment_data.heartbeat_mode;
	}

	return {
		subscribe,
		set,
		update,
		getHeartbeatMode
	};
}

export const environment = createEnvironment();
