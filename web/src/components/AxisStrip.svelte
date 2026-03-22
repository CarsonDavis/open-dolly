<script lang="ts">
	import type { AxisCapability } from '@opendolly/shared';
	import { formatAxisValue } from '$lib/utils/format';

	interface Props {
		axis: AxisCapability;
		value: number;
		disabled?: boolean;
		onmoveto?: (value: number) => void;
		onjog?: (delta: number) => void;
	}

	let { axis, value, disabled = false, onmoveto, onjog }: Props = $props();

	let mode = $state<'jog' | 'position'>('jog');
	let isEditing = $state(false);
	let inputValue = $state('');
	let isDragging = $state(false);
	let jogElement: HTMLDivElement | undefined = $state();

	let displayName = $derived(axis.name.charAt(0).toUpperCase() + axis.name.slice(1));
	let isLinear = $derived(axis.unit === 'mm');
	let fineStep = $derived(isLinear ? 1 : 1);
	let coarseStep = $derived(isLinear ? 10 : 15);
	let unitSymbol = $derived(axis.unit === 'deg' ? '\u00B0' : 'mm');

	function startEditing() {
		isEditing = true;
		inputValue = value.toFixed(1);
	}

	function submitValue(e: Event) {
		e.preventDefault();
		const target = parseFloat(inputValue);
		if (!isNaN(target)) {
			const clamped = Math.round(Math.min(axis.max, Math.max(axis.min, target)) * 10) / 10;
			onmoveto?.(clamped);
		}
		isEditing = false;
	}

	function increment(amount: number) {
		const target = Math.round(Math.min(axis.max, Math.max(axis.min, value + amount)) * 10) / 10;
		onmoveto?.(target);
	}

	// Jog drag handling
	function handleJogStart(e: PointerEvent) {
		if (disabled || mode !== 'jog') return;
		isDragging = true;
		(e.target as HTMLElement).setPointerCapture(e.pointerId);
		handleJogMove(e);
	}

	function handleJogMove(e: PointerEvent) {
		if (!isDragging || !jogElement) return;
		const rect = jogElement.getBoundingClientRect();
		const cx = rect.width / 2;
		const relX = (e.clientX - rect.left - cx) / cx; // -1 to 1
		const clamped = Math.max(-1, Math.min(1, relX));
		const deadZone = 0.05;
		if (Math.abs(clamped) < deadZone) return;
		const adjusted = Math.sign(clamped) * ((Math.abs(clamped) - deadZone) / (1 - deadZone));
		const delta = adjusted * (axis.max - axis.min) * 0.01;
		onjog?.(delta);
	}

	function handleJogEnd() {
		if (!isDragging) return;
		isDragging = false;
		onjog?.(0); // zero-velocity stop
	}

	// Position bar click
	function handlePositionClick(e: MouseEvent) {
		if (disabled || mode !== 'position') return;
		const target = e.currentTarget as HTMLElement;
		const rect = target.getBoundingClientRect();
		const frac = (e.clientX - rect.left) / rect.width;
		const pos = Math.round((axis.min + frac * (axis.max - axis.min)) * 10) / 10;
		onmoveto?.(pos);
	}

	let positionFraction = $derived(
		axis.max !== axis.min ? (value - axis.min) / (axis.max - axis.min) : 0
	);
</script>

<div class="axis-strip" class:disabled>
	<div class="strip-header">
		<span class="axis-name">{displayName}</span>
		{#if isEditing}
			<form class="inline-edit" onsubmit={submitValue}>
				<input
					type="number"
					bind:value={inputValue}
					min={axis.min}
					max={axis.max}
					step="0.1"
					class="edit-input"
				/>
				<button type="submit" class="edit-go">Go</button>
			</form>
		{:else}
			<button class="value-readout" onclick={startEditing} {disabled}>
				{formatAxisValue(value, axis)}
			</button>
		{/if}
		<button
			class="mode-toggle"
			class:active={mode === 'position'}
			onclick={() => mode = mode === 'jog' ? 'position' : 'jog'}
			{disabled}
		>
			{mode === 'jog' ? 'Jog' : 'Pos'}
		</button>
	</div>

	<div class="strip-controls">
		<button class="inc-btn" onclick={() => increment(-coarseStep)} {disabled}>-{coarseStep}</button>
		<button class="inc-btn" onclick={() => increment(-fineStep)} {disabled}>-{fineStep}</button>

		{#if mode === 'jog'}
			<!-- svelte-ignore a11y_no_static_element_interactions -->
			<div
				class="jog-bar"
				class:dragging={isDragging}
				bind:this={jogElement}
				onpointerdown={handleJogStart}
				onpointermove={handleJogMove}
				onpointerup={handleJogEnd}
				onpointercancel={handleJogEnd}
			>
				<div class="jog-center"></div>
				<span class="jog-label">{'\u25C0'} drag {'\u25B6'}</span>
			</div>
		{:else}
			<!-- svelte-ignore a11y_click_events_have_key_events -->
			<!-- svelte-ignore a11y_no_static_element_interactions -->
			<div class="position-bar" onclick={handlePositionClick}>
				<div class="position-fill" style="width: {positionFraction * 100}%"></div>
				<div class="position-marker" style="left: {positionFraction * 100}%"></div>
			</div>
		{/if}

		<button class="inc-btn" onclick={() => increment(fineStep)} {disabled}>+{fineStep}</button>
		<button class="inc-btn" onclick={() => increment(coarseStep)} {disabled}>+{coarseStep}</button>
	</div>
</div>

<style>
	.axis-strip {
		padding: 0.375rem 0;
	}

	.axis-strip.disabled {
		opacity: 0.5;
		pointer-events: none;
	}

	.strip-header {
		display: flex;
		align-items: center;
		gap: 0.5rem;
		margin-bottom: 0.25rem;
	}

	.axis-name {
		font-size: 0.75rem;
		color: var(--color-text-muted);
		text-transform: capitalize;
		min-width: 3rem;
	}

	.value-readout {
		font-family: var(--font-mono);
		font-size: 0.875rem;
		color: var(--color-text);
		padding: 0.125rem 0.375rem;
		min-height: auto;
		min-width: auto;
		border-radius: 4px;
		flex: 1;
		text-align: left;
	}

	.value-readout:hover {
		background: var(--color-primary);
	}

	.inline-edit {
		display: flex;
		gap: 0.25rem;
		align-items: center;
		flex: 1;
	}

	.edit-input {
		width: 5rem;
		font-family: var(--font-mono);
		font-size: 0.8rem;
		padding: 0.125rem 0.375rem;
		min-height: 1.75rem;
	}

	.edit-go {
		padding: 0.125rem 0.375rem;
		background: var(--color-primary);
		border-radius: 4px;
		font-size: 0.75rem;
		min-height: 1.75rem;
		min-width: auto;
	}

	.mode-toggle {
		font-size: 0.65rem;
		padding: 0.125rem 0.375rem;
		min-height: auto;
		min-width: auto;
		border-radius: 4px;
		color: var(--color-text-muted);
		text-transform: uppercase;
		letter-spacing: 0.05em;
	}

	.mode-toggle.active {
		color: var(--color-accent);
	}

	.strip-controls {
		display: flex;
		align-items: center;
		gap: 0.25rem;
	}

	.inc-btn {
		font-size: 0.7rem;
		font-family: var(--font-mono);
		padding: 0.25rem 0.375rem;
		min-height: 2rem;
		min-width: auto;
		border-radius: 4px;
		color: var(--color-text-muted);
		white-space: nowrap;
	}

	.inc-btn:hover:not(:disabled) {
		background: var(--color-primary);
		color: var(--color-text);
	}

	.jog-bar {
		flex: 1;
		height: 2rem;
		background: var(--color-surface);
		border: 1px solid var(--color-primary);
		border-radius: var(--radius);
		position: relative;
		touch-action: none;
		cursor: ew-resize;
		display: flex;
		align-items: center;
		justify-content: center;
	}

	.jog-bar.dragging {
		border-color: var(--color-accent);
	}

	.jog-center {
		position: absolute;
		left: 50%;
		top: 25%;
		bottom: 25%;
		width: 1px;
		background: var(--color-primary);
	}

	.jog-label {
		font-size: 0.6rem;
		color: var(--color-text-muted);
		pointer-events: none;
		opacity: 0.5;
	}

	.position-bar {
		flex: 1;
		height: 2rem;
		background: var(--color-surface);
		border: 1px solid var(--color-primary);
		border-radius: var(--radius);
		position: relative;
		cursor: pointer;
		overflow: hidden;
	}

	.position-fill {
		position: absolute;
		top: 0;
		left: 0;
		bottom: 0;
		background: var(--color-primary);
		opacity: 0.3;
	}

	.position-marker {
		position: absolute;
		top: 2px;
		bottom: 2px;
		width: 3px;
		background: var(--color-accent);
		border-radius: 1px;
		transform: translateX(-50%);
	}
</style>
