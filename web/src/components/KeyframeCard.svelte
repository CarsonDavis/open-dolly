<script lang="ts">
	import type { AxisCapability } from '@opendolly/shared';
	import type { Keyframe } from '$lib/stores/keyframes';
	import { formatPositionSummary } from '$lib/utils/format';

	interface Props {
		keyframe: Keyframe;
		index: number;
		axes: AxisCapability[];
		isFirst: boolean;
		isLast: boolean;
		ongoto?: () => void;
		ondelete?: () => void;
		onmoveup?: () => void;
		onmovedown?: () => void;
		onupdatelabel?: (label: string) => void;
	}

	let {
		keyframe,
		index,
		axes,
		isFirst,
		isLast,
		ongoto,
		ondelete,
		onmoveup,
		onmovedown,
		onupdatelabel
	}: Props = $props();

	let isEditingLabel = $state(false);
	let labelInput = $state('');

	function submitLabel() {
		if (labelInput.trim() && labelInput !== keyframe.label) {
			onupdatelabel?.(labelInput.trim());
		}
		isEditingLabel = false;
	}
</script>

<div class="keyframe-card">
	<div class="card-header">
		{#if isEditingLabel}
			<form onsubmit={(e) => { e.preventDefault(); submitLabel(); }}>
				<input
					type="text"
					bind:value={labelInput}
					onblur={submitLabel}
					class="label-input"
				/>
			</form>
		{:else}
			<button class="label" onclick={() => { isEditingLabel = true; labelInput = keyframe.label; }}>
				{keyframe.label}
			</button>
		{/if}

		<div class="card-actions">
			<button class="reorder-btn" onclick={onmoveup} disabled={isFirst} aria-label="Move up">&#9650;</button>
			<button class="reorder-btn" onclick={onmovedown} disabled={isLast} aria-label="Move down">&#9660;</button>
		</div>
	</div>

	<div class="positions">
		{formatPositionSummary(keyframe.positions, axes)}
	</div>

	<div class="card-footer">
		<button class="action-btn goto" onclick={ongoto}>Go To</button>
		<button class="action-btn delete" onclick={ondelete}>Delete</button>
	</div>
</div>

<style>
	.keyframe-card {
		background: var(--color-surface);
		border: 1px solid var(--color-primary);
		border-radius: var(--radius);
		padding: 0.75rem;
	}

	.card-header {
		display: flex;
		align-items: center;
		justify-content: space-between;
		margin-bottom: 0.375rem;
	}

	.label {
		font-weight: 600;
		font-size: 0.875rem;
		min-height: auto;
		min-width: auto;
		padding: 0.125rem 0.25rem;
		border-radius: 4px;
	}

	.label:hover {
		background: var(--color-primary);
	}

	.label-input {
		font-size: 0.875rem;
		font-weight: 600;
		padding: 0.125rem 0.5rem;
		min-height: 1.75rem;
		width: 10rem;
	}

	.card-actions {
		display: flex;
		gap: 0.125rem;
	}

	.reorder-btn {
		font-size: 0.7rem;
		padding: 0.125rem 0.375rem;
		min-height: auto;
		min-width: auto;
		color: var(--color-text-muted);
		border-radius: 4px;
	}

	.reorder-btn:hover:not(:disabled) {
		background: var(--color-primary);
		color: var(--color-text);
	}

	.reorder-btn:disabled {
		opacity: 0.3;
		cursor: default;
	}

	.positions {
		font-size: 0.8rem;
		color: var(--color-text-muted);
		font-family: var(--font-mono);
		margin-bottom: 0.5rem;
	}

	.card-footer {
		display: flex;
		gap: 0.5rem;
	}

	.action-btn {
		font-size: 0.8rem;
		padding: 0.375rem 0.75rem;
		border-radius: var(--radius);
		min-height: 2rem;
		min-width: auto;
	}

	.goto {
		background: var(--color-primary);
	}

	.goto:hover {
		background: var(--color-accent);
	}

	.delete {
		color: var(--color-error);
	}

	.delete:hover {
		background: rgba(244, 67, 54, 0.15);
	}
</style>
