<script lang="ts">
	import type { TrajectoryPoint } from '@opendolly/shared';
	import type { AxisInterpolation } from '@opendolly/motion-math';
	import { sequenceStore } from '$lib/stores/sequence';
	import { trajectoryStore } from '$lib/stores/trajectory';
	import { stateStore, isIdle, isPlaying } from '$lib/stores/state';
	import { connectionStore, isConnected } from '$lib/stores/connection';
	import { capabilitiesStore } from '$lib/stores/capabilities';
	import { BoardClient } from '$lib/api/client';
	import { buildTrajectory, getSequenceDuration, allTransitionsSet } from '$lib/utils/trajectory-builder';

	let hasEnoughKeyframes = $derived($sequenceStore.keyframes.length >= 2);
	let transitionsReady = $derived(hasEnoughKeyframes && allTransitionsSet($sequenceStore));
	let canUploadAndPlay = $derived(transitionsReady && $isConnected && $isIdle);

	async function uploadAndPlay() {
		if (!canUploadAndPlay) return;
		const caps = $capabilitiesStore;
		if (!caps) return;

		const ws = connectionStore.getWebSocket();
		if (!ws) return;

		trajectoryStore.setComputing();
		try {
			// Build axis type map from capabilities
			const rotationAxes = new Set(['pan', 'tilt', 'roll']);
			const axisTypes: Record<string, AxisInterpolation> = {};
			for (const axis of caps.axes) {
				axisTypes[axis.name] = rotationAxes.has(axis.name) ? 'rotation' : 'linear';
			}

			// Build trajectory from sequence
			const result = buildTrajectory($sequenceStore, axisTypes);
			const points = result.points as TrajectoryPoint[];
			const durationMs = getSequenceDuration($sequenceStore);
			trajectoryStore.setComputed(points, durationMs);

			// Upload
			trajectoryStore.setUploading();
			const baseUrl = import.meta.env.VITE_BOARD_URL ?? '';
			const client = new BoardClient(baseUrl);
			const uploadResult = await client.uploadTrajectory({ points, loop: false });

			trajectoryStore.setUploaded(uploadResult.trajectory_id);
			stateStore.update((s) => ({ ...s, trajectoryId: uploadResult.trajectory_id }));

			// Play
			ws.sendPlay(uploadResult.trajectory_id);
		} catch (err) {
			trajectoryStore.setError(err instanceof Error ? err.message : 'Upload failed');
		}
	}

	function handlePause() {
		connectionStore.getWebSocket()?.sendPause();
	}

	function handleResume() {
		connectionStore.getWebSocket()?.sendResume();
	}

	function handleStop() {
		connectionStore.getWebSocket()?.sendStop();
	}
</script>

<div class="playback-controls">
	{#if $stateStore.state === 'playing' || $stateStore.state === 'paused'}
		<div class="transport">
			{#if $stateStore.state === 'playing'}
				<button class="transport-btn pause" onclick={handlePause}>Pause</button>
			{:else}
				<button class="transport-btn resume" onclick={handleResume}>Resume</button>
			{/if}
			<button class="transport-btn stop" onclick={handleStop}>Stop</button>

			{#if $stateStore.playbackProgress != null}
				<div class="progress-bar">
					<div class="progress-fill" style="width: {($stateStore.playbackProgress ?? 0) * 100}%"></div>
				</div>
			{/if}
		</div>
	{:else if $stateStore.state === 'moving'}
		<div class="transport">
			<button class="transport-btn stop" onclick={handleStop}>Stop</button>
			<span class="status-text">Moving...</span>
		</div>
	{:else}
		<button
			class="upload-play-btn"
			onclick={uploadAndPlay}
			disabled={!canUploadAndPlay}
		>
			{#if !hasEnoughKeyframes}
				Need 2+ keyframes
			{:else if !transitionsReady}
				Set transition durations
			{:else if $trajectoryStore.state === 'computing'}
				Computing...
			{:else if $trajectoryStore.state === 'uploading'}
				Uploading...
			{:else}
				Upload & Play
			{/if}
		</button>
	{/if}

	{#if $trajectoryStore.state === 'error'}
		<div class="error-msg">{$trajectoryStore.error}</div>
	{/if}

	{#if $stateStore.error}
		<div class="error-msg">
			{$stateStore.error.code}: {$stateStore.error.detail}
			{#if $stateStore.error.axis}(axis: {$stateStore.error.axis}){/if}
		</div>
	{/if}
</div>

<style>
	.playback-controls {
		display: flex;
		flex-direction: column;
		gap: 0.75rem;
		padding: 0.75rem;
		background: var(--color-surface);
		border-radius: var(--radius);
		border: 1px solid var(--color-primary);
	}

	.upload-play-btn {
		background: var(--color-accent);
		border-radius: var(--radius);
		padding: 0.75rem;
		font-size: 0.9rem;
		font-weight: 600;
		width: 100%;
	}

	.upload-play-btn:hover:not(:disabled) {
		filter: brightness(1.1);
	}

	.upload-play-btn:disabled {
		opacity: 0.4;
		cursor: default;
	}

	.transport {
		display: flex;
		gap: 0.5rem;
		align-items: center;
		flex-wrap: wrap;
	}

	.transport-btn {
		padding: 0.5rem 1rem;
		border-radius: var(--radius);
		font-size: 0.875rem;
		font-weight: 500;
		min-width: auto;
	}

	.transport-btn.pause {
		background: var(--color-warning);
		color: #000;
	}

	.transport-btn.resume {
		background: var(--color-success);
		color: #000;
	}

	.transport-btn.stop {
		background: var(--color-error);
	}

	.status-text {
		font-size: 0.875rem;
		color: var(--color-text-muted);
	}

	.progress-bar {
		flex: 1;
		min-width: 100px;
		height: 6px;
		background: var(--color-primary);
		border-radius: 3px;
		overflow: hidden;
	}

	.progress-fill {
		height: 100%;
		background: var(--color-accent);
		transition: width 0.1s linear;
	}

	.error-msg {
		font-size: 0.8rem;
		color: var(--color-error);
		padding: 0.5rem;
		background: rgba(244, 67, 54, 0.1);
		border-radius: var(--radius);
	}
</style>
