<script lang="ts">
	import type { AxisCapability } from '@slider/shared';
	import { connectionStore, isConnected } from '$lib/stores/connection';
	import { positionStore } from '$lib/stores/position';
	import AxisControl from './AxisControl.svelte';

	interface Props {
		axes: AxisCapability[];
	}

	let { axes }: Props = $props();

	let padElement: HTMLDivElement | undefined = $state();
	let isDragging = $state(false);

	// Primary axes mapped to X/Y on the touch pad — default to first two available
	let xAxisName = $state('');
	let yAxisName = $state('');

	$effect(() => {
		if (!xAxisName && axes.length > 0) {
			xAxisName = axes.find((a) => a.name === 'pan')?.name ?? axes[0].name;
		}
		if (!yAxisName && axes.length > 0) {
			yAxisName = axes.find((a) => a.name === 'slide')?.name ?? axes[1]?.name ?? axes[0].name;
		}
	});

	let xAxis = $derived(axes.find((a) => a.name === xAxisName));
	let yAxis = $derived(axes.find((a) => a.name === yAxisName));

	// Axis options for the switcher
	let axisOptions = $derived(axes.map((a) => a.name));

	const DEAD_ZONE = 0.05; // 5% of pad radius

	function getPadPosition(e: Touch | MouseEvent): { x: number; y: number } | null {
		if (!padElement) return null;
		const rect = padElement.getBoundingClientRect();
		const cx = rect.width / 2;
		const cy = rect.height / 2;
		const x = ((e.clientX - rect.left) - cx) / cx; // -1 to 1
		const y = ((e.clientY - rect.top) - cy) / cy;  // -1 to 1
		return { x, y };
	}

	function applyDeadZone(v: number): number {
		const abs = Math.abs(v);
		if (abs < DEAD_ZONE) return 0;
		return Math.sign(v) * ((abs - DEAD_ZONE) / (1 - DEAD_ZONE));
	}

	function sendJog(pos: { x: number; y: number }) {
		const ws = connectionStore.getWebSocket();
		if (!ws) return;
		const jogAxes: Record<string, number> = {};
		const xVal = applyDeadZone(Math.max(-1, Math.min(1, pos.x)));
		const yVal = applyDeadZone(Math.max(-1, Math.min(1, pos.y)));
		if (xAxis && xVal !== 0) jogAxes[xAxisName] = xVal * (xAxis.max - xAxis.min) * 0.01;
		if (yAxis && yVal !== 0) jogAxes[yAxisName] = -yVal * (yAxis.max - yAxis.min) * 0.01;
		if (Object.keys(jogAxes).length > 0) ws.sendJog(jogAxes);
	}

	function handleTouchStart(e: TouchEvent) {
		if (!$isConnected) return;
		e.preventDefault();
		isDragging = true;
		const pos = getPadPosition(e.touches[0]);
		if (pos) sendJog(pos);
	}

	function handleTouchMove(e: TouchEvent) {
		if (!isDragging) return;
		e.preventDefault();
		const pos = getPadPosition(e.touches[0]);
		if (pos) sendJog(pos);
	}

	function stopJog() {
		const ws = connectionStore.getWebSocket();
		if (!ws) return;
		// Send zero-velocity jog for each active axis to trigger deceleration
		const stopAxes: Record<string, number> = {};
		if (xAxis) stopAxes[xAxisName] = 0;
		if (yAxis) stopAxes[yAxisName] = 0;
		ws.sendJog(stopAxes);
	}

	function handleTouchEnd() {
		isDragging = false;
		stopJog();
	}

	function handleMouseDown(e: MouseEvent) {
		if (!$isConnected) return;
		isDragging = true;
		const pos = getPadPosition(e);
		if (pos) sendJog(pos);
	}

	function handleMouseMove(e: MouseEvent) {
		if (!isDragging) return;
		const pos = getPadPosition(e);
		if (pos) sendJog(pos);
	}

	function handleMouseUp() {
		if (!isDragging) return;
		isDragging = false;
		stopJog();
	}

	function handleAxisJog(detail: { axis: string; delta: number }) {
		const ws = connectionStore.getWebSocket();
		if (ws) ws.sendJog({ [detail.axis]: detail.delta });
	}

	function handleAxisMoveTo(detail: { axis: string; value: number }) {
		const ws = connectionStore.getWebSocket();
		if (ws) ws.sendMoveTo({ [detail.axis]: detail.value }, 1000);
	}
</script>

<svelte:window onmouseup={handleMouseUp} onmousemove={handleMouseMove} />

<div class="jog-control">
	<div class="pad-header">
		<div class="axis-selector">
			<label>
				X: <select bind:value={xAxisName}>
					{#each axisOptions as name}
						<option value={name}>{name}</option>
					{/each}
				</select>
			</label>
			<label>
				Y: <select bind:value={yAxisName}>
					{#each axisOptions as name}
						<option value={name}>{name}</option>
					{/each}
				</select>
			</label>
		</div>
	</div>

	<!-- svelte-ignore a11y_no_noninteractive_element_interactions -->
	<div
		class="pad"
		class:disabled={!$isConnected}
		class:dragging={isDragging}
		bind:this={padElement}
		role="application"
		aria-label="Manual axis control. Use touch or mouse drag to jog."
		ontouchstart={handleTouchStart}
		ontouchmove={handleTouchMove}
		ontouchend={handleTouchEnd}
		onmousedown={handleMouseDown}
	>
		<div class="crosshair-h"></div>
		<div class="crosshair-v"></div>
		<div class="center-dot"></div>
		<div class="axis-label-x">{xAxisName}</div>
		<div class="axis-label-y">{yAxisName}</div>
	</div>

	<div class="axis-readouts">
		{#each axes as axis (axis.name)}
			<AxisControl
				{axis}
				value={$positionStore[axis.name] ?? 0}
				disabled={!$isConnected}
				onjog={handleAxisJog}
				onmoveto={handleAxisMoveTo}
			/>
		{/each}
	</div>
</div>

<style>
	.jog-control {
		display: flex;
		flex-direction: column;
		gap: 0.75rem;
	}

	.pad-header {
		display: flex;
		justify-content: center;
	}

	.axis-selector {
		display: flex;
		gap: 1rem;
		font-size: 0.8rem;
		color: var(--color-text-muted);
	}

	.axis-selector label {
		display: flex;
		align-items: center;
		gap: 0.25rem;
	}

	.axis-selector select {
		font-size: 0.8rem;
		padding: 0.25rem 0.5rem;
		min-height: 2rem;
		text-transform: capitalize;
	}

	.pad {
		position: relative;
		width: 100%;
		aspect-ratio: 1;
		max-width: 300px;
		margin: 0 auto;
		background: var(--color-surface);
		border: 2px solid var(--color-primary);
		border-radius: var(--radius);
		touch-action: none;
		user-select: none;
		cursor: crosshair;
	}

	.pad.disabled {
		opacity: 0.4;
		pointer-events: none;
	}

	.pad.dragging {
		border-color: var(--color-accent);
	}

	.crosshair-h,
	.crosshair-v {
		position: absolute;
		background: var(--color-primary);
	}

	.crosshair-h {
		top: 50%;
		left: 0;
		right: 0;
		height: 1px;
	}

	.crosshair-v {
		left: 50%;
		top: 0;
		bottom: 0;
		width: 1px;
	}

	.center-dot {
		position: absolute;
		top: 50%;
		left: 50%;
		width: 10px;
		height: 10px;
		border-radius: 50%;
		background: var(--color-accent);
		transform: translate(-50%, -50%);
		opacity: 0.6;
	}

	.axis-label-x,
	.axis-label-y {
		position: absolute;
		font-size: 0.7rem;
		color: var(--color-text-muted);
		text-transform: capitalize;
	}

	.axis-label-x {
		bottom: 4px;
		right: 8px;
	}

	.axis-label-y {
		top: 4px;
		left: 8px;
	}

	.axis-readouts {
		display: flex;
		flex-direction: column;
		gap: 0.25rem;
	}
</style>
