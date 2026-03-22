<script lang="ts">
	import type { AxisCapability } from '@opendolly/shared';
	import { connectionStore, isConnected } from '$lib/stores/connection';
	import { positionStore } from '$lib/stores/position';
	import { sequenceStore, selectedKeyframeId } from '$lib/stores/sequence';
	import AxisStrip from './AxisStrip.svelte';

	interface Props {
		axes: AxisCapability[];
	}

	let { axes }: Props = $props();

	let collapsed = $state(false);
	let interactiveMode = $state(true);

	let hasPositionData = $derived(Object.keys($positionStore).length > 0);
	let selectedKf = $derived(
		$selectedKeyframeId
			? $sequenceStore.keyframes.find((kf) => kf.id === $selectedKeyframeId)
			: null
	);

	function captureKeyframe() {
		if (!hasPositionData) return;
		sequenceStore.addKeyframe({ ...$positionStore });
	}

	function deselectKeyframe() {
		selectedKeyframeId.set(null);
	}

	function handleMoveTo(axisName: string, value: number) {
		if (interactiveMode) {
			const ws = connectionStore.getWebSocket();
			if (ws) ws.sendMoveTo({ [axisName]: value }, 1000);
		}
		// If a keyframe is selected, update its position
		if ($selectedKeyframeId) {
			const currentPositions = selectedKf?.positions ?? {};
			sequenceStore.updateKeyframePositions($selectedKeyframeId, {
				...currentPositions,
				[axisName]: value
			});
		}
	}

	function handleJog(axisName: string, delta: number) {
		if (!interactiveMode) return;
		const ws = connectionStore.getWebSocket();
		if (ws) ws.sendJog({ [axisName]: delta });
	}

	function getAxisValue(axisName: string): number {
		if ($selectedKeyframeId && selectedKf) {
			return selectedKf.positions[axisName] ?? 0;
		}
		return $positionStore[axisName] ?? 0;
	}
</script>

{#if collapsed}
	<div class="control-panel-collapsed">
		{#if $selectedKeyframeId}
			<span class="editing-label">Editing {selectedKf?.label}</span>
			<button class="deselect-btn" onclick={deselectKeyframe}>Done</button>
		{:else}
			<button
				class="capture-btn-mini"
				onclick={captureKeyframe}
				disabled={!$isConnected || !hasPositionData}
			>
				Capture Keyframe
			</button>
		{/if}
		<button class="expand-btn" onclick={() => collapsed = false}>&#9650;</button>
	</div>
{:else}
	<div class="control-panel">
		<div class="panel-header">
			<div class="mode-toggle">
				<button
					class="mode-btn"
					class:active={interactiveMode}
					onclick={() => interactiveMode = true}
				>Interactive</button>
				<button
					class="mode-btn"
					class:active={!interactiveMode}
					onclick={() => interactiveMode = false}
				>Static</button>
			</div>
		</div>

		<div class="axis-strips">
			{#each axes as axis (axis.name)}
				<AxisStrip
					{axis}
					value={getAxisValue(axis.name)}
					disabled={!$isConnected && interactiveMode}
					onmoveto={(v) => handleMoveTo(axis.name, v)}
					onjog={(d) => handleJog(axis.name, d)}
				/>
			{/each}
		</div>

		<div class="panel-footer">
			{#if $selectedKeyframeId}
				<span class="editing-label">Editing {selectedKf?.label}</span>
				<button class="deselect-btn" onclick={deselectKeyframe}>Done</button>
			{:else}
				<button
					class="capture-btn"
					onclick={captureKeyframe}
					disabled={!$isConnected || !hasPositionData}
				>
					Capture Keyframe
				</button>
			{/if}
			<button class="collapse-btn" onclick={() => collapsed = true}>&#9660; minimize</button>
		</div>
	</div>
{/if}

<style>
	.control-panel {
		display: flex;
		flex-direction: column;
		gap: 0.5rem;
		padding: 0.75rem;
		background: var(--color-surface);
		border-radius: var(--radius);
		border: 1px solid var(--color-primary);
	}

	.control-panel-collapsed {
		display: flex;
		align-items: center;
		gap: 0.5rem;
		padding: 0.5rem 0.75rem;
		background: var(--color-surface);
		border-radius: var(--radius);
		border: 1px solid var(--color-primary);
	}

	.panel-header {
		display: flex;
		justify-content: center;
	}

	.mode-toggle {
		display: flex;
		gap: 0;
		border: 1px solid var(--color-primary);
		border-radius: var(--radius);
		overflow: hidden;
	}

	.mode-btn {
		font-size: 0.7rem;
		padding: 0.25rem 0.75rem;
		min-height: auto;
		min-width: auto;
		border-radius: 0;
		color: var(--color-text-muted);
		text-transform: uppercase;
		letter-spacing: 0.05em;
	}

	.mode-btn.active {
		background: var(--color-primary);
		color: var(--color-text);
	}

	.axis-strips {
		display: flex;
		flex-direction: column;
	}

	.panel-footer {
		display: flex;
		flex-direction: column;
		gap: 0.375rem;
		align-items: center;
	}

	.capture-btn,
	.capture-btn-mini {
		background: var(--color-primary);
		border-radius: var(--radius);
		padding: 0.625rem;
		font-size: 0.875rem;
		font-weight: 500;
		width: 100%;
	}

	.capture-btn:hover:not(:disabled),
	.capture-btn-mini:hover:not(:disabled) {
		background: var(--color-accent);
	}

	.capture-btn:disabled,
	.capture-btn-mini:disabled {
		opacity: 0.4;
		cursor: default;
	}

	.capture-btn-mini {
		flex: 1;
	}

	.editing-label {
		font-size: 0.8rem;
		color: var(--color-accent);
		font-weight: 500;
		flex: 1;
	}

	.deselect-btn {
		font-size: 0.8rem;
		padding: 0.375rem 0.75rem;
		min-height: auto;
		min-width: auto;
		border-radius: var(--radius);
		background: var(--color-primary);
	}

	.collapse-btn,
	.expand-btn {
		font-size: 0.7rem;
		color: var(--color-text-muted);
		min-height: auto;
		min-width: auto;
		padding: 0.125rem 0.5rem;
	}
</style>
