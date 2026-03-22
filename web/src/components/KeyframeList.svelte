<script lang="ts">
	import type { AxisCapability } from '@opendolly/shared';
	import { sequenceStore, selectedKeyframeId } from '$lib/stores/sequence';
	import { connectionStore, isConnected } from '$lib/stores/connection';
	import KeyframeCard from './KeyframeCard.svelte';
	import TransitionEditor from './TransitionEditor.svelte';

	interface Props {
		axes: AxisCapability[];
	}

	let { axes }: Props = $props();

	let editingTransitionId = $state<string | null>(null);

	function goToKeyframe(id: string, positions: Record<string, number>) {
		// Select the keyframe and move hardware there
		selectedKeyframeId.set(id);
		const ws = connectionStore.getWebSocket();
		if (ws) ws.sendMoveTo(positions, 1000);
	}

	function handleCardClick(id: string, positions: Record<string, number>) {
		if ($selectedKeyframeId === id) {
			// Deselect
			selectedKeyframeId.set(null);
		} else {
			goToKeyframe(id, positions);
		}
	}
</script>

<div class="keyframe-list">
	{#if $sequenceStore.keyframes.length === 0}
		<div class="empty">
			<p>No keyframes yet.</p>
			<p class="hint">Position the camera, then capture a keyframe.</p>
		</div>
	{:else}
		<div class="cards">
			{#each $sequenceStore.keyframes as kf, i (kf.id)}
				<KeyframeCard
					keyframe={kf}
					index={i}
					{axes}
					isFirst={i === 0}
					isLast={i === $sequenceStore.keyframes.length - 1}
					selected={$selectedKeyframeId === kf.id}
					ongoto={() => handleCardClick(kf.id, kf.positions)}
					ondelete={() => {
						if ($selectedKeyframeId === kf.id) selectedKeyframeId.set(null);
						sequenceStore.removeKeyframe(kf.id);
					}}
					onmoveup={() => sequenceStore.reorderKeyframe(i, i - 1)}
					onmovedown={() => sequenceStore.reorderKeyframe(i, i + 1)}
					onupdatelabel={(label) => sequenceStore.updateKeyframeLabel(kf.id, label)}
				/>

				{#if i < $sequenceStore.transitions.length}
					{@const tr = $sequenceStore.transitions[i]}
					{#if editingTransitionId === tr.id}
						<TransitionEditor
							transition={tr}
							keyframeA={kf}
							keyframeB={$sequenceStore.keyframes[i + 1]}
							{axes}
							onclose={() => editingTransitionId = null}
						/>
					{:else}
						<button
							class="duration-bracket"
							class:unset={tr.duration_ms <= 0}
							onclick={() => editingTransitionId = tr.id}
						>
							{tr.duration_ms > 0 ? (tr.duration_ms / 1000).toFixed(1) + 's' : '—'}
						</button>
					{/if}
				{/if}
			{/each}
		</div>
	{/if}
</div>

<style>
	.keyframe-list {
		display: flex;
		flex-direction: column;
		gap: 0;
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
		gap: 0;
	}

	.duration-bracket {
		display: flex;
		align-items: center;
		justify-content: center;
		padding: 0.25rem 0;
		margin: 0.125rem 2rem;
		font-family: var(--font-mono);
		font-size: 0.75rem;
		color: var(--color-text-muted);
		min-height: auto;
		min-width: auto;
		border-radius: 4px;
		position: relative;
	}

	.duration-bracket::before,
	.duration-bracket::after {
		content: '';
		position: absolute;
		left: 50%;
		width: 1px;
		background: var(--color-primary);
	}

	.duration-bracket::before {
		top: 0;
		height: 4px;
	}

	.duration-bracket::after {
		bottom: 0;
		height: 4px;
	}

	.duration-bracket:hover {
		background: var(--color-primary);
		color: var(--color-text);
	}

	.duration-bracket.unset {
		color: var(--color-accent);
		animation: pulse-text 1.5s ease-in-out infinite;
	}

	@keyframes pulse-text {
		0%, 100% { opacity: 1; }
		50% { opacity: 0.4; }
	}
</style>
