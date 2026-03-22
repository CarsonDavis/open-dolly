<script lang="ts">
	import type { AxisCapability } from '@opendolly/shared';
	import { sequenceStore, type SequenceKeyframe, type SequenceTransition } from '$lib/stores/sequence';
	import CurveEditor from './CurveEditor.svelte';

	interface Props {
		transition: SequenceTransition;
		keyframeA: SequenceKeyframe;
		keyframeB: SequenceKeyframe;
		axes: AxisCapability[];
		onclose?: () => void;
	}

	let { transition, keyframeA, keyframeB, axes, onclose }: Props = $props();

	let speedPrimaryAxis = $state<string | null>(null);
	let showCurveEditor = $state(false);

	// Compute per-axis deltas and speeds
	interface AxisDelta {
		name: string;
		delta: number;
		unit: string;
		speedUnit: string;
		speed: number | null;
	}

	let axisDeltas = $derived.by(() => {
		const deltas: AxisDelta[] = [];
		for (const axis of axes) {
			const a = keyframeA.positions[axis.name] ?? 0;
			const b = keyframeB.positions[axis.name] ?? 0;
			const delta = Math.abs(b - a);
			if (delta < 0.05) continue; // skip axes with no meaningful change
			const speedUnit = axis.unit === 'mm' ? 'mm/s' : '\u00B0/s';
			const durationSec = transition.duration_ms / 1000;
			const speed = durationSec > 0 ? Math.round((delta / durationSec) * 10) / 10 : null;
			deltas.push({ name: axis.name, delta, unit: axis.unit, speedUnit, speed });
		}
		return deltas;
	});

	let durationSec = $derived(transition.duration_ms / 1000);

	function handleDurationChange(e: Event) {
		const target = e.target as HTMLInputElement;
		const val = parseFloat(target.value);
		if (!isNaN(val) && val > 0) {
			speedPrimaryAxis = null;
			sequenceStore.updateTransition(transition.id, {
				duration_ms: Math.round(val * 1000)
			});
		}
	}

	function handleSpeedChange(axisName: string, e: Event) {
		const target = e.target as HTMLInputElement;
		const speed = parseFloat(target.value);
		if (isNaN(speed) || speed <= 0) return;

		const axisDelta = axisDeltas.find((d) => d.name === axisName);
		if (!axisDelta) return;

		const newDurationSec = axisDelta.delta / speed;
		const newDurationMs = Math.round(newDurationSec * 1000);
		if (newDurationMs > 0) {
			sequenceStore.updateTransition(transition.id, { duration_ms: newDurationMs });
		}
	}

	function selectSpeedPrimary(axisName: string) {
		speedPrimaryAxis = axisName;
	}

	function backToDuration() {
		speedPrimaryAxis = null;
	}

	function handleClose() {
		if (transition.duration_ms <= 0) return; // blocking — can't close without duration
		onclose?.();
	}

	let hasAdvancedCurves = $derived(
		Object.values(transition.curves).some((c) => c.points.length > 0)
	);
</script>

<div class="transition-editor">
	<div class="te-header">
		<span class="te-label">
			{keyframeA.label} → {keyframeB.label}
		</span>
		<button
			class="te-close"
			onclick={handleClose}
			disabled={transition.duration_ms <= 0}
		>&times;</button>
	</div>

	{#if speedPrimaryAxis}
		{@const axisDelta = axisDeltas.find((d) => d.name === speedPrimaryAxis)}
		<div class="te-field">
			<span class="te-field-label">Duration</span>
			<span class="te-derived">{durationSec > 0 ? durationSec.toFixed(1) : '—'} s</span>
		</div>
		{#if axisDelta}
			<div class="te-field">
				<label class="te-field-label">
					{axisDelta.name.charAt(0).toUpperCase() + axisDelta.name.slice(1)} speed
				</label>
				<div class="te-input-group">
					<input
						type="number"
						value={axisDelta.speed ?? ''}
						min="0.1"
						step="0.1"
						class="te-input"
						oninput={(e) => handleSpeedChange(speedPrimaryAxis!, e)}
					/>
					<span class="te-unit">{axisDelta.speedUnit}</span>
				</div>
			</div>
		{/if}
		<button class="te-link" onclick={backToDuration}>Back to duration</button>
	{:else}
		<div class="te-field">
			<label class="te-field-label">Duration</label>
			<div class="te-input-group">
				<input
					type="number"
					value={durationSec > 0 ? durationSec : ''}
					placeholder="required"
					min="0.1"
					step="0.1"
					class="te-input"
					class:required={transition.duration_ms <= 0}
					oninput={handleDurationChange}
				/>
				<span class="te-unit">s</span>
			</div>
		</div>

		{#if axisDeltas.length > 0 && transition.duration_ms > 0}
			<div class="te-speeds">
				{#each axisDeltas as ad}
					<button class="te-speed-row" onclick={() => selectSpeedPrimary(ad.name)}>
						<span class="te-speed-axis">{ad.name.charAt(0).toUpperCase() + ad.name.slice(1)}</span>
						<span class="te-speed-value">
							{ad.speed !== null ? ad.speed.toFixed(1) : '—'} {ad.speedUnit}
						</span>
						<span class="te-speed-arrow">&#9664;</span>
					</button>
				{/each}
			</div>
		{/if}
	{/if}

	{#if showCurveEditor}
		<CurveEditor
			{transition}
			{keyframeA}
			{keyframeB}
			{axes}
			ondone={() => showCurveEditor = false}
		/>
	{:else}
		<button
			class="te-advanced"
			onclick={() => showCurveEditor = true}
			disabled={transition.duration_ms <= 0}
		>
			Advanced {hasAdvancedCurves ? '(edited)' : ''}
		</button>
	{/if}
</div>

<style>
	.transition-editor {
		display: flex;
		flex-direction: column;
		gap: 0.5rem;
		padding: 0.625rem;
		background: var(--color-surface);
		border: 1px solid var(--color-accent);
		border-radius: var(--radius);
	}

	.te-header {
		display: flex;
		align-items: center;
		justify-content: space-between;
	}

	.te-label {
		font-size: 0.75rem;
		color: var(--color-text-muted);
		font-weight: 500;
	}

	.te-close {
		font-size: 1.2rem;
		min-height: auto;
		min-width: auto;
		padding: 0 0.25rem;
		color: var(--color-text-muted);
		line-height: 1;
	}

	.te-close:disabled {
		opacity: 0.3;
		cursor: not-allowed;
	}

	.te-field {
		display: flex;
		align-items: center;
		justify-content: space-between;
		gap: 0.5rem;
	}

	.te-field-label {
		font-size: 0.8rem;
		color: var(--color-text-muted);
	}

	.te-input-group {
		display: flex;
		align-items: center;
		gap: 0.25rem;
	}

	.te-input {
		width: 5rem;
		font-family: var(--font-mono);
		font-size: 0.875rem;
		padding: 0.25rem 0.5rem;
		min-height: 2rem;
	}

	.te-input.required {
		border-color: var(--color-accent);
		animation: pulse-border 1.5s ease-in-out infinite;
	}

	@keyframes pulse-border {
		0%, 100% { border-color: var(--color-accent); }
		50% { border-color: var(--color-error); }
	}

	.te-unit {
		font-size: 0.8rem;
		color: var(--color-text-muted);
	}

	.te-derived {
		font-family: var(--font-mono);
		font-size: 0.875rem;
		color: var(--color-text-muted);
	}

	.te-speeds {
		display: flex;
		flex-direction: column;
		gap: 0.125rem;
	}

	.te-speed-row {
		display: flex;
		align-items: center;
		gap: 0.5rem;
		padding: 0.25rem 0.375rem;
		min-height: auto;
		min-width: auto;
		border-radius: 4px;
		font-size: 0.8rem;
		text-align: left;
	}

	.te-speed-row:hover {
		background: var(--color-primary);
	}

	.te-speed-axis {
		color: var(--color-text-muted);
		min-width: 3rem;
	}

	.te-speed-value {
		font-family: var(--font-mono);
		color: var(--color-text-muted);
		flex: 1;
	}

	.te-speed-arrow {
		font-size: 0.6rem;
		color: var(--color-text-muted);
		opacity: 0.5;
	}

	.te-link {
		font-size: 0.75rem;
		color: var(--color-accent);
		min-height: auto;
		min-width: auto;
		padding: 0.125rem;
		text-align: left;
	}

	.te-advanced {
		font-size: 0.8rem;
		padding: 0.375rem;
		min-height: auto;
		border-radius: var(--radius);
		color: var(--color-text-muted);
		border: 1px solid var(--color-primary);
	}

	.te-advanced:hover:not(:disabled) {
		background: var(--color-primary);
		color: var(--color-text);
	}

	.te-advanced:disabled {
		opacity: 0.3;
	}
</style>
