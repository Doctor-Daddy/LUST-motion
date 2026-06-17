<script lang="ts">
	import type { PageData } from './$types';
	import Device from './Device.svelte';
	import Control from './Control.svelte';

	interface Props {
		data: PageData;
	}

	let { data }: Props = $props();
	let chartHeight = $state(1.0);
	let chartDurationSeconds = $state(10);
	let chartSpeedRange = $state(4100);

	let chartDurationMs = $derived(chartDurationSeconds * 1000);
</script>

<div
	class="mx-0 my-1 flex flex-col space-y-4
     sm:mx-8 sm:my-8"
>
	<Device 
	chartHeight={chartHeight}
	chartDurationMs={chartDurationMs}
	chartSpeedRange={chartSpeedRange}
	/>

	<Control />

	<div class="card bg-base-200 shadow-md shadow-primary/50 mb-1.5 mx-auto w-11/12 p-4"> <!-- Start Display Settings DIV -->
		<h2 class="text-xl font-semibold mb-4">Display Settings</h2>

		<!-- Chart Height (Aspect Ratio) -->
		<div class="mt-4">
			<input
				type="range"
				min="0.5"
				max="3"
				step="0.1"
				bind:value={chartHeight}
				class="range range-primary range-xs w-full"
			/>

			<label class="input mt-2 w-full" for="chart_height">
				<input
					id="chart_height"
					type="number"
					min="0.5"
					max="3"
					step="0.1"
					bind:value={chartHeight}
				/>
				<span class="label">x</span>
			</label>
		</div>

		<!-- Chart Time Duration -->
		<div class="mt-4">
		<input
			type="range"
			min="5"
			max="60"
			step="5"
			bind:value={chartDurationSeconds}
			class="range range-primary range-xs w-full"
		/>

		<label class="input mt-2 w-full" for="chart_duration">
			<input
				id="chart_duration"
				type="number"
				min="5"
				max="60"
				step="5"
				bind:value={chartDurationSeconds}
			/>
			<span class="label">s</span>
		</label>
	</div>

	<!-- Chart Velocity Range (Magnitude) -->
	<div class="mt-4">
		<input
			type="range"
			min="50"
			max="10000"
			step="50"
			bind:value={chartSpeedRange}
			class="range range-primary range-xs w-full"
		/>

		<label class="input mt-2 w-full" for="chart_speed_range">
			<input
				id="chart_speed_range"
				type="number"
				min="50"
				max="10000"
				step="50"
				bind:value={chartSpeedRange}
			/>
			<span class="label">mm/s</span>
		</label>
	</div>

	</div> <!-- End Display Settings DIV -->

</div>
