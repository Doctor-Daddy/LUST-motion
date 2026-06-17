<script lang="ts">
	interface Props {
		chartHeight?: number;
		chartDurationMs?: number;
		chartSpeedRange?: number;
	}

	let {
		chartHeight = 1.0,
		chartDurationMs = 10000,
		chartSpeedRange = 4100
	}: Props = $props();

	const BASE_CHART_HEIGHT = 368;
	let chartContainerHeight = $derived(`${Math.round(BASE_CHART_HEIGHT * chartHeight)}px`);

	import type { PageData } from './$types';
	import { page } from '$app/state';
	import { safeStateHeartbeat } from '$lib/stores/safestate';
	import { socket } from '$lib/stores/socket';
	import { Chart, registerables } from 'chart.js';
	import * as LuxonAdapter from 'chartjs-adapter-luxon';
	import ChartStreaming from '@robloche/chartjs-plugin-streaming';
	import { onMount, onDestroy } from 'svelte';
	import Go from '~icons/tabler/refresh-alert';
	import { daisyColor } from '$lib/DaisyUiHelper';

	Chart.register(...registerables);
	Chart.register(LuxonAdapter);
	Chart.register(ChartStreaming);

	let positionChartElement: HTMLCanvasElement;
	let positionChart: Chart;

	let timeSync: number = 0;

	function clearSafeState() {
		$safeStateHeartbeat.safestate = false;
		safeStateHeartbeat.sendSafeState();
	}

	onDestroy(() => {
		socket.off('rawdata');
	});

	onMount(() => {
		connectDataSocket();
		createChart();
	});

	function connectDataSocket() {
		socket.on('rawdata', (data) => {
			if (timeSync == 0) {
				timeSync = Date.now() - data.rawdata[0][0];
			}

			for (let i = 0; i < data.rawdata.length; i++) {
				positionChart.data.datasets[0].data.push({
					x: timeSync + data.rawdata[i][0],
					y: data.rawdata[i][1]
				});
				positionChart.data.datasets[1].data.push({
					x: timeSync + data.rawdata[i][0],
					y: data.rawdata[i][2]
				});
			}
		});
	}

	function createChart() {
		positionChart = new Chart(positionChartElement, {
			type: 'line',
			data: {
				datasets: [
					{
						// Position
						borderColor: daisyColor('--color-primary'),
						fill: false,
						pointRadius: 0,
						data: [],
						yAxisID: 'y'
					},
					{
						// Speed
						backgroundColor: daisyColor('--color-secondary', 20),
						borderColor: daisyColor('--color-secondary'),
						fill: true,
						pointRadius: 0,
						data: [],
						yAxisID: 'y1'
					}
				]
			},
			options: {
				animation: false,
				responsive: true,
				maintainAspectRatio: false,
				interaction: {
					mode: 'index',
					intersect: false
				},
				plugins: {
					// Change options for ALL axes of THIS CHART
					streaming: {
						duration: chartDurationMs,
						refresh: 50,
						delay: 100
					},
					tooltip: {
						enabled: false
					},
					legend: {
						display: false
					}
				},
				scales: {
					x: {
						type: 'realtime',
						grid: {
							color: daisyColor('--color-base-content', 10)
						},
						ticks: { color: daisyColor('--color-base-content') }
					},
					y: {
						type: 'linear',
						title: {
							display: true,
							text: 'Position [mm]',
							color: daisyColor('--color-primary'),
							font: {
								size: 16,
								weight: 'bold'
							}
						},
						position: 'left',
						min: 0,
						max: 225,
						grid: { color: daisyColor('--color-base-content', 10) },
						ticks: {
							stepSize: 25,
							color: daisyColor('--color-base-content')
						},
						border: { color: daisyColor('--color-base-content', 10) }
					},
					y1: {
						type: 'linear',
						title: {
							display: true,
							text: 'Speed [mm/s]',
							color: daisyColor('--color-secondary'),
							font: {
								size: 16,
								weight: 'bold'
							}
						},
						position: 'right',
						suggestedMin: -chartSpeedRange,
						suggestedMax: chartSpeedRange,
						ticks: {
							stepSize: Math.round(chartSpeedRange / 4),
							color: daisyColor('--color-base-content')
						},
						grid: {
							drawOnChartArea: false // only want the grid lines for one axis to show up
						},
						border: { color: daisyColor('--color-base-content', 10) }
					}
				}
			}
		});
	}

	$effect(() => {
		if (!positionChart) return;

		const scales = positionChart.options.scales as any;
		scales.x.realtime.duration = chartDurationMs;
		scales.y1.suggestedMin = -chartSpeedRange;
		scales.y1.suggestedMax = chartSpeedRange;
		scales.y1.ticks.stepSize = Math.round(chartSpeedRange / 4);

		positionChart.resize();
		positionChart.update('none');
	});

</script>

<div class="card bg-base-200 shadow-md shadow-primary/50 mx-auto w-11/12">
	{#if $safeStateHeartbeat.safestate}
		<div class="flex p-4">
			<a
				class="sm:text-8xl text-6xl hover:animate-pulse active:scale-[0.98]"
				onclick={clearSafeState}
			>
				<Go class="text-success h-full w-auto" />
			</a>

			<div class="flex h-full w-full flex-col items-center justify-center p-6">
				<p class="lg:text-4xl sm:text-3xl text-2xl font-bold text-error text-center">
					Device in Safe State
				</p>
				<p class="text-center">Clear safe state to enable output</p>
			</div>
		</div>
	{:else}
		<div class="relative w-full p-2" style:height={chartContainerHeight}>
			<canvas bind:this={positionChartElement}></canvas>
		</div>
	{/if}
</div>
