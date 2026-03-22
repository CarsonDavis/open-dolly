<script lang="ts">
	import type { AxisCapability } from '@opendolly/shared';
	import type { ProgressCurvePoint } from '@opendolly/motion-math';
	import { sequenceStore, type SequenceKeyframe, type SequenceTransition } from '$lib/stores/sequence';
	import CurveLane from './CurveLane.svelte';

	interface Props {
		transition: SequenceTransition;
		keyframeA: SequenceKeyframe;
		keyframeB: SequenceKeyframe;
		axes: AxisCapability[];
		ondone?: () => void;
	}

	let { transition, keyframeA, keyframeB, axes, ondone }: Props = $props();

	let linkedAxes = $state(new Set<string>());

	let durationSec = $derived(transition.duration_ms / 1000);

	// Only show axes with a delta
	let activeAxes = $derived.by(() => {
		return axes.filter((axis) => {
			const a = keyframeA.positions[axis.name] ?? 0;
			const b = keyframeB.positions[axis.name] ?? 0;
			return Math.abs(b - a) >= 0.05;
		});
	});

	function getPoints(axisName: string): ProgressCurvePoint[] {
		return transition.curves[axisName]?.points ?? [];
	}

	function handlePointsChange(axisName: string, newPoints: ProgressCurvePoint[]) {
		const curves = { ...transition.curves };
		curves[axisName] = { points: newPoints };

		// If linked, apply same points to all linked axes
		if (linkedAxes.has(axisName)) {
			for (const linkedAxis of linkedAxes) {
				if (linkedAxis !== axisName) {
					curves[linkedAxis] = { points: [...newPoints.map((p) => ({ ...p }))] };
				}
			}
		}

		sequenceStore.updateTransition(transition.id, { curves });
	}

	function handleLinkChange(axisName: string, linked: boolean) {
		const next = new Set(linkedAxes);
		if (linked) {
			next.add(axisName);
		} else {
			next.delete(axisName);
		}
		linkedAxes = next;
	}

	// Easing presets
	type PresetName = 'linear' | 'easeIn' | 'easeOut' | 'easeInOut';

	const presetPoints: Record<PresetName, ProgressCurvePoint[]> = {
		linear: [],
		easeIn: [{ t: 0.4, progress: 0.1 }],
		easeOut: [{ t: 0.6, progress: 0.9 }],
		easeInOut: [{ t: 0.3, progress: 0.05 }, { t: 0.7, progress: 0.95 }]
	};

	function applyPreset(preset: PresetName) {
		const targetAxes = linkedAxes.size > 0
			? [...linkedAxes]
			: activeAxes.map((a) => a.name);

		const curves = { ...transition.curves };
		for (const axisName of targetAxes) {
			curves[axisName] = { points: presetPoints[preset].map((p) => ({ ...p })) };
		}
		sequenceStore.updateTransition(transition.id, { curves });
	}
</script>

<div class="curve-editor">
	<div class="ce-header">
		<span class="ce-title">{keyframeA.label} → {keyframeB.label} ({durationSec.toFixed(1)}s)</span>
		<button class="ce-done" onclick={ondone}>Done</button>
	</div>

	<div class="ce-presets">
		<button class="preset-btn" onclick={() => applyPreset('linear')}>Linear</button>
		<button class="preset-btn" onclick={() => applyPreset('easeIn')}>Ease In</button>
		<button class="preset-btn" onclick={() => applyPreset('easeOut')}>Ease Out</button>
		<button class="preset-btn" onclick={() => applyPreset('easeInOut')}>Ease In-Out</button>
	</div>

	<div class="ce-lanes">
		{#each activeAxes as axis (axis.name)}
			<CurveLane
				axisName={axis.name}
				points={getPoints(axis.name)}
				{durationSec}
				linked={linkedAxes.has(axis.name)}
				onpointschange={(pts) => handlePointsChange(axis.name, pts)}
				onlinkchange={(v) => handleLinkChange(axis.name, v)}
			/>
		{/each}

		{#if activeAxes.length === 0}
			<div class="ce-empty">No axes change between these keyframes.</div>
		{/if}
	</div>
</div>

<style>
	.curve-editor {
		display: flex;
		flex-direction: column;
		gap: 0.5rem;
		padding: 0.625rem;
		background: var(--color-surface);
		border: 1px solid var(--color-accent);
		border-radius: var(--radius);
	}

	.ce-header {
		display: flex;
		align-items: center;
		justify-content: space-between;
	}

	.ce-title {
		font-size: 0.75rem;
		color: var(--color-text-muted);
		font-weight: 500;
	}

	.ce-done {
		font-size: 0.8rem;
		padding: 0.25rem 0.75rem;
		min-height: auto;
		min-width: auto;
		border-radius: var(--radius);
		background: var(--color-primary);
	}

	.ce-done:hover {
		background: var(--color-accent);
	}

	.ce-presets {
		display: flex;
		gap: 0.25rem;
		flex-wrap: wrap;
	}

	.preset-btn {
		font-size: 0.65rem;
		padding: 0.25rem 0.5rem;
		min-height: auto;
		min-width: auto;
		border-radius: 4px;
		color: var(--color-text-muted);
		border: 1px solid var(--color-primary);
		text-transform: uppercase;
		letter-spacing: 0.03em;
	}

	.preset-btn:hover {
		background: var(--color-primary);
		color: var(--color-text);
	}

	.ce-lanes {
		display: flex;
		flex-direction: column;
		gap: 0.25rem;
	}

	.ce-empty {
		font-size: 0.8rem;
		color: var(--color-text-muted);
		text-align: center;
		padding: 1rem;
	}
</style>
