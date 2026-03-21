<script lang="ts">
	import type { AxisCapability } from '@slider/shared';
	import { keyframeStore } from '$lib/stores/keyframes';
	import { positionStore } from '$lib/stores/position';
	import { connectionStore, isConnected } from '$lib/stores/connection';
	import KeyframeCard from './KeyframeCard.svelte';

	interface Props {
		axes: AxisCapability[];
	}

	let { axes }: Props = $props();

	let hasPositionData = $derived(Object.keys($positionStore).length > 0);

	function capturePosition() {
		if (!hasPositionData) return;
		keyframeStore.add({ ...$positionStore });
	}

	function goToKeyframe(positions: Record<string, number>) {
		const ws = connectionStore.getWebSocket();
		if (ws) ws.sendMoveTo(positions, 1000);
	}
</script>

<div class="keyframe-list">
	{#if $keyframeStore.length === 0}
		<div class="empty">
			<p>No keyframes saved.</p>
			<p class="hint">Jog the camera to a position, then capture it as a keyframe.</p>
		</div>
	{:else}
		<div class="cards">
			{#each $keyframeStore as kf, i (kf.id)}
				<KeyframeCard
					keyframe={kf}
					index={i}
					{axes}
					isFirst={i === 0}
					isLast={i === $keyframeStore.length - 1}
					ongoto={() => goToKeyframe(kf.positions)}
					ondelete={() => keyframeStore.remove(kf.id)}
					onmoveup={() => keyframeStore.reorder(i, i - 1)}
					onmovedown={() => keyframeStore.reorder(i, i + 1)}
					onupdatelabel={(label) => keyframeStore.updateLabel(kf.id, label)}
				/>
			{/each}
		</div>
	{/if}

	<button class="capture-btn" onclick={capturePosition} disabled={!$isConnected || !hasPositionData}>
		+ Capture Current Position
	</button>
</div>

<style>
	.keyframe-list {
		display: flex;
		flex-direction: column;
		gap: 0.75rem;
	}

	.empty {
		text-align: center;
		padding: 2rem 1rem;
		color: var(--color-text-muted);
	}

	.empty p {
		margin-bottom: 0.5rem;
	}

	.hint {
		font-size: 0.8rem;
	}

	.cards {
		display: flex;
		flex-direction: column;
		gap: 0.5rem;
	}

	.capture-btn {
		background: var(--color-primary);
		border-radius: var(--radius);
		padding: 0.75rem;
		font-size: 0.875rem;
		font-weight: 500;
		width: 100%;
	}

	.capture-btn:hover:not(:disabled) {
		background: var(--color-accent);
	}

	.capture-btn:disabled {
		opacity: 0.4;
		cursor: default;
	}
</style>
