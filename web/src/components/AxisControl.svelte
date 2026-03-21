<script lang="ts">
	import type { AxisCapability } from '@opendolly/shared';
	import { formatAxisValue } from '$lib/utils/format';

	interface Props {
		axis: AxisCapability;
		value: number;
		disabled?: boolean;
		onjog?: (detail: { axis: string; delta: number }) => void;
		onmoveto?: (detail: { axis: string; value: number }) => void;
	}

	let { axis, value, disabled = false, onjog, onmoveto }: Props = $props();
	let inputValue = $state('');
	let isEditing = $state(false);

	function handleInput(e: Event) {
		const target = e.target as HTMLInputElement;
		const newValue = parseFloat(target.value);
		if (!isNaN(newValue) && onmoveto) {
			onmoveto({ axis: axis.name, value: newValue });
		}
	}

	function handleManualSubmit(e: Event) {
		e.preventDefault();
		const target = parseFloat(inputValue);
		if (!isNaN(target) && target >= axis.min && target <= axis.max && onmoveto) {
			onmoveto({ axis: axis.name, value: target });
		}
		isEditing = false;
		inputValue = '';
	}

	function startEditing() {
		isEditing = true;
		inputValue = value.toFixed(1);
	}

	let unitLabel = $derived(axis.unit === 'deg' ? '\u00B0' : axis.unit);
	let displayName = $derived(axis.name.charAt(0).toUpperCase() + axis.name.slice(1));
</script>

<div class="axis-control" class:disabled>
	<div class="axis-header">
		<span class="axis-label">{displayName} ({axis.unit})</span>
		{#if isEditing}
			<form class="manual-input" onsubmit={handleManualSubmit}>
				<input
					type="number"
					bind:value={inputValue}
					min={axis.min}
					max={axis.max}
					step="0.1"
					class="axis-input"
					{disabled}
				/>
				<button type="submit" class="go-btn" {disabled}>Go</button>
			</form>
		{:else}
			<button class="value-display" onclick={startEditing} {disabled}>
				{formatAxisValue(value, axis)}
			</button>
		{/if}
	</div>
	<div class="slider-row">
		<span class="range-label">{axis.min}</span>
		<input
			type="range"
			min={axis.min}
			max={axis.max}
			step="0.1"
			value={value}
			oninput={handleInput}
			{disabled}
			class="axis-slider"
			aria-label="{displayName} position"
				aria-valuemin={axis.min}
			aria-valuemax={axis.max}
			aria-valuenow={value}
		/>
		<span class="range-label">{axis.max}</span>
	</div>
</div>

<style>
	.axis-control {
		padding: 0.5rem 0;
	}

	.axis-control.disabled {
		opacity: 0.5;
		pointer-events: none;
	}

	.axis-header {
		display: flex;
		align-items: center;
		justify-content: space-between;
		margin-bottom: 0.25rem;
	}

	.axis-label {
		font-size: 0.8rem;
		color: var(--color-text-muted);
		text-transform: capitalize;
	}

	.value-display {
		font-family: var(--font-mono);
		font-size: 0.875rem;
		color: var(--color-text);
		padding: 0.125rem 0.5rem;
		min-height: auto;
		min-width: auto;
		border-radius: 4px;
	}

	.value-display:hover {
		background: var(--color-primary);
	}

	.manual-input {
		display: flex;
		gap: 0.25rem;
		align-items: center;
	}

	.axis-input {
		width: 5rem;
		font-family: var(--font-mono);
		font-size: 0.8rem;
		padding: 0.25rem 0.5rem;
		min-height: 2rem;
	}

	.go-btn {
		padding: 0.25rem 0.5rem;
		background: var(--color-primary);
		border-radius: var(--radius);
		font-size: 0.75rem;
		min-height: 2rem;
		min-width: auto;
	}

	.slider-row {
		display: flex;
		align-items: center;
		gap: 0.5rem;
	}

	.range-label {
		font-size: 0.7rem;
		color: var(--color-text-muted);
		font-family: var(--font-mono);
		min-width: 2.5rem;
		text-align: center;
	}

	.axis-slider {
		flex: 1;
		accent-color: var(--color-accent);
		height: 4px;
		min-height: auto;
		border: none;
		padding: 0;
	}
</style>
