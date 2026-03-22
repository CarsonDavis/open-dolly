<script lang="ts">
	import type { AxisCapability } from '@opendolly/shared';
	import type { AxisInterpolation } from '@opendolly/motion-math';
	import { sequenceStore } from '$lib/stores/sequence';
	import { connectionStore, isConnected } from '$lib/stores/connection';
	import { capabilitiesStore } from '$lib/stores/capabilities';
	import { trajectoryStore } from '$lib/stores/trajectory';
	import { stateStore } from '$lib/stores/state';
	import { BoardClient } from '$lib/api/client';
	import { buildTrajectory, getSequenceDuration, allTransitionsSet } from '$lib/utils/trajectory-builder';
	import CurveLane from './CurveLane.svelte';

	interface Props {
		axes: AxisCapability[];
	}

	let { axes }: Props = $props();

	let timelineElement: HTMLDivElement | undefined = $state();
	let isDraggingPlayhead = $state(false);
	let isDraggingKeyframe = $state<number | null>(null);
	let playheadTimeMs = $state(0);
	let expandedLane = $state<{ transitionIdx: number; axisName: string } | null>(null);

	// Auto-upload state
	let uploadedTrajectoryId = $state<string | null>(null);
	let uploadDebounceTimer = $state<ReturnType<typeof setTimeout> | null>(null);
	let isSyncing = $state(false);

	let totalDurationMs = $derived(getSequenceDuration($sequenceStore));
	let totalDurationSec = $derived(totalDurationMs / 1000);
	let hasValidSequence = $derived(
		$sequenceStore.keyframes.length >= 2 && allTransitionsSet($sequenceStore)
	);

	// Compute cumulative times for keyframe positions
	let keyframeTimes = $derived.by(() => {
		const times = [0];
		let cumulative = 0;
		for (const tr of $sequenceStore.transitions) {
			cumulative += tr.duration_ms;
			times.push(cumulative);
		}
		return times;
	});

	// Axes with any delta across the sequence
	let activeAxes = $derived.by(() => {
		if ($sequenceStore.keyframes.length < 2) return [];
		return axes.filter((axis) => {
			const values = $sequenceStore.keyframes.map((kf) => kf.positions[axis.name] ?? 0);
			const min = Math.min(...values);
			const max = Math.max(...values);
			return max - min >= 0.05;
		});
	});

	// Convert pixel X to time in ms
	function xToTime(clientX: number): number {
		if (!timelineElement || totalDurationMs <= 0) return 0;
		const rect = timelineElement.getBoundingClientRect();
		const frac = Math.max(0, Math.min(1, (clientX - rect.left) / rect.width));
		return Math.round(frac * totalDurationMs);
	}

	// Convert time to percentage
	function timeToPercent(timeMs: number): number {
		return totalDurationMs > 0 ? (timeMs / totalDurationMs) * 100 : 0;
	}

	// Playhead scrub
	let scrubThrottleTimer = $state<ReturnType<typeof setTimeout> | null>(null);

	function handlePlayheadDown(e: PointerEvent) {
		if (!hasValidSequence) return;
		isDraggingPlayhead = true;
		(e.target as HTMLElement).setPointerCapture(e.pointerId);
		playheadTimeMs = xToTime(e.clientX);
		sendScrub(playheadTimeMs);
	}

	function handleTimelinePointerMove(e: PointerEvent) {
		if (isDraggingPlayhead) {
			playheadTimeMs = xToTime(e.clientX);
			// Throttle scrub to 20Hz
			if (!scrubThrottleTimer) {
				sendScrub(playheadTimeMs);
				scrubThrottleTimer = setTimeout(() => { scrubThrottleTimer = null; }, 50);
			}
		} else if (isDraggingKeyframe !== null) {
			handleKeyframeDrag(e);
		}
	}

	function handleTimelinePointerUp() {
		if (isDraggingPlayhead) {
			sendScrub(playheadTimeMs);
		}
		isDraggingPlayhead = false;
		isDraggingKeyframe = null;
	}

	function sendScrub(timeMs: number) {
		if (!uploadedTrajectoryId) return;
		const ws = connectionStore.getWebSocket();
		if (ws && $isConnected) {
			ws.sendScrub(timeMs);
		}
	}

	// Keyframe drag-to-retime
	function handleKeyframeDown(idx: number, e: PointerEvent) {
		if (idx === 0 || idx === $sequenceStore.keyframes.length - 1) return; // can't drag endpoints
		e.stopPropagation();
		isDraggingKeyframe = idx;
		(e.target as HTMLElement).setPointerCapture(e.pointerId);
	}

	function handleKeyframeDrag(e: PointerEvent) {
		if (isDraggingKeyframe === null) return;
		const idx = isDraggingKeyframe;
		const newTimeMs = xToTime(e.clientX);

		// Clamp between previous and next keyframe times (min 100ms gap)
		const prevTime = keyframeTimes[idx - 1] + 100;
		const nextTime = keyframeTimes[idx + 1] - 100;
		const clampedTime = Math.max(prevTime, Math.min(nextTime, newTimeMs));

		// Redistribute durations
		const leftDuration = clampedTime - keyframeTimes[idx - 1];
		const rightDuration = keyframeTimes[idx + 1] - clampedTime;

		const leftTr = $sequenceStore.transitions[idx - 1];
		const rightTr = $sequenceStore.transitions[idx];

		if (leftTr && rightTr) {
			sequenceStore.updateTransition(leftTr.id, { duration_ms: leftDuration });
			sequenceStore.updateTransition(rightTr.id, { duration_ms: rightDuration });
		}
	}

	// Auto-upload trajectory for scrub
	$effect(() => {
		// Trigger on sequence changes
		const _ = $sequenceStore;
		if (!hasValidSequence || !$isConnected) return;

		if (uploadDebounceTimer) clearTimeout(uploadDebounceTimer);
		isSyncing = true;

		uploadDebounceTimer = setTimeout(async () => {
			try {
				const caps = $capabilitiesStore;
				if (!caps) return;

				const rotationAxes = new Set(['pan', 'tilt', 'roll']);
				const axisTypes: Record<string, AxisInterpolation> = {};
				for (const axis of caps.axes) {
					axisTypes[axis.name] = rotationAxes.has(axis.name) ? 'rotation' : 'linear';
				}

				const built = buildTrajectory($sequenceStore, axisTypes);
				if (built.points.length === 0) return;

				const baseUrl = import.meta.env.VITE_BOARD_URL ?? '';
				const client = new BoardClient(baseUrl);
				const uploadResult = await client.uploadTrajectory({ points: built.points, loop: false });
				uploadedTrajectoryId = uploadResult.trajectory_id;
			} catch {
				// Silent fail — scrub just won't work until next upload
			} finally {
				isSyncing = false;
			}
		}, 500);
	});

	// Expand/collapse lane for inline editing
	function toggleLane(transitionIdx: number, axisName: string) {
		if (expandedLane?.transitionIdx === transitionIdx && expandedLane?.axisName === axisName) {
			expandedLane = null;
		} else {
			expandedLane = { transitionIdx, axisName };
		}
	}
</script>

<div
	class="timeline-view"
	bind:this={timelineElement}
	onpointermove={handleTimelinePointerMove}
	onpointerup={handleTimelinePointerUp}
>
	{#if !hasValidSequence}
		<div class="tl-empty">Need 2+ keyframes with durations set.</div>
	{:else}
		<!-- Sync indicator -->
		{#if isSyncing}
			<div class="tl-syncing">Syncing...</div>
		{/if}

		<!-- Keyframe track -->
		<div class="tl-keyframe-track">
			{#each $sequenceStore.keyframes as kf, i (kf.id)}
				{@const percent = timeToPercent(keyframeTimes[i])}
				<!-- svelte-ignore a11y_no_static_element_interactions -->
				<div
					class="tl-kf-marker"
					class:draggable={i > 0 && i < $sequenceStore.keyframes.length - 1}
					style="left: {percent}%"
					onpointerdown={(e) => handleKeyframeDown(i, e)}
				>
					<div class="tl-kf-line"></div>
					<span class="tl-kf-label">{kf.label}</span>
				</div>
			{/each}

			<!-- Duration labels between markers -->
			{#each $sequenceStore.transitions as tr, i}
				{@const leftPercent = timeToPercent(keyframeTimes[i])}
				{@const rightPercent = timeToPercent(keyframeTimes[i + 1])}
				{@const centerPercent = (leftPercent + rightPercent) / 2}
				<span class="tl-duration-label" style="left: {centerPercent}%">
					{(tr.duration_ms / 1000).toFixed(1)}s
				</span>
			{/each}
		</div>

		<!-- Axis lanes -->
		<div class="tl-lanes">
			{#each activeAxes as axis (axis.name)}
				<div class="tl-lane">
					<span class="tl-lane-label">{axis.name}</span>
					<div class="tl-lane-segments">
						{#each $sequenceStore.transitions as tr, i}
							{@const leftPercent = timeToPercent(keyframeTimes[i])}
							{@const width = timeToPercent(tr.duration_ms)}
							{@const isExpanded = expandedLane?.transitionIdx === i && expandedLane?.axisName === axis.name}
							<!-- svelte-ignore a11y_click_events_have_key_events -->
							<!-- svelte-ignore a11y_no_static_element_interactions -->
							<div
								class="tl-lane-segment"
								class:expanded={isExpanded}
								style="left: {leftPercent}%; width: {width}%"
								onclick={() => toggleLane(i, axis.name)}
							>
								{#if isExpanded}
									<CurveLane
										axisName={axis.name}
										points={tr.curves[axis.name]?.points ?? []}
										durationSec={tr.duration_ms / 1000}
										onpointschange={(pts) => {
											const curves = { ...tr.curves };
											curves[axis.name] = { points: pts };
											sequenceStore.updateTransition(tr.id, { curves });
										}}
									/>
								{:else}
									<CurveLane
										axisName={axis.name}
										points={tr.curves[axis.name]?.points ?? []}
										durationSec={tr.duration_ms / 1000}
										compact
									/>
								{/if}
							</div>
						{/each}
					</div>
				</div>
			{/each}
		</div>

		<!-- Playhead -->
		<!-- svelte-ignore a11y_no_static_element_interactions -->
		<div
			class="tl-playhead"
			class:dragging={isDraggingPlayhead}
			style="left: {timeToPercent(playheadTimeMs)}%"
			onpointerdown={handlePlayheadDown}
		>
			<div class="tl-playhead-line"></div>
			<div class="tl-playhead-handle"></div>
		</div>

		<!-- Time scale -->
		<div class="tl-time-scale">
			{#each Array(Math.min(Math.ceil(totalDurationSec), 20) + 1) as _, i}
				{@const percent = timeToPercent(i * 1000)}
				{#if percent <= 100}
					<span class="tl-tick" style="left: {percent}%">{i}s</span>
				{/if}
			{/each}
		</div>
	{/if}
</div>

<style>
	.timeline-view {
		position: relative;
		background: var(--color-surface);
		border: 1px solid var(--color-primary);
		border-radius: var(--radius);
		padding: 0.5rem 0.75rem;
		min-height: 120px;
		touch-action: none;
		user-select: none;
	}

	.tl-empty {
		display: flex;
		align-items: center;
		justify-content: center;
		height: 80px;
		color: var(--color-text-muted);
		font-size: 0.8rem;
	}

	.tl-syncing {
		position: absolute;
		top: 4px;
		right: 8px;
		font-size: 0.65rem;
		color: var(--color-warning);
	}

	/* Keyframe track */
	.tl-keyframe-track {
		position: relative;
		height: 24px;
		margin-bottom: 0.25rem;
	}

	.tl-kf-marker {
		position: absolute;
		top: 0;
		transform: translateX(-50%);
		display: flex;
		flex-direction: column;
		align-items: center;
		z-index: 2;
	}

	.tl-kf-marker.draggable {
		cursor: ew-resize;
	}

	.tl-kf-line {
		width: 2px;
		height: 10px;
		background: var(--color-text);
	}

	.tl-kf-label {
		font-size: 0.55rem;
		color: var(--color-text-muted);
		white-space: nowrap;
		max-width: 60px;
		overflow: hidden;
		text-overflow: ellipsis;
	}

	.tl-duration-label {
		position: absolute;
		top: 2px;
		transform: translateX(-50%);
		font-size: 0.6rem;
		font-family: var(--font-mono);
		color: var(--color-text-muted);
	}

	/* Axis lanes */
	.tl-lanes {
		display: flex;
		flex-direction: column;
		gap: 2px;
	}

	.tl-lane {
		display: flex;
		align-items: stretch;
		min-height: 30px;
	}

	.tl-lane-label {
		font-size: 0.6rem;
		color: var(--color-text-muted);
		text-transform: capitalize;
		width: 2.5rem;
		flex-shrink: 0;
		display: flex;
		align-items: center;
	}

	.tl-lane-segments {
		flex: 1;
		position: relative;
		min-height: 30px;
	}

	.tl-lane-segment {
		position: absolute;
		top: 0;
		bottom: 0;
		border: 1px solid var(--color-primary);
		border-radius: 2px;
		overflow: hidden;
		cursor: pointer;
	}

	.tl-lane-segment:hover {
		border-color: var(--color-accent);
	}

	.tl-lane-segment.expanded {
		min-height: 120px;
		z-index: 1;
		background: var(--color-surface);
		border-color: var(--color-accent);
	}

	/* Playhead */
	.tl-playhead {
		position: absolute;
		top: 0;
		bottom: 0;
		transform: translateX(-50%);
		z-index: 10;
		cursor: ew-resize;
		width: 12px;
	}

	.tl-playhead-line {
		position: absolute;
		left: 50%;
		top: 0;
		bottom: 0;
		width: 2px;
		transform: translateX(-50%);
		background: var(--color-accent);
		opacity: 0.7;
	}

	.tl-playhead.dragging .tl-playhead-line {
		opacity: 1;
	}

	.tl-playhead-handle {
		position: absolute;
		top: -2px;
		left: 50%;
		transform: translateX(-50%);
		width: 10px;
		height: 10px;
		background: var(--color-accent);
		border-radius: 50%;
	}

	/* Time scale */
	.tl-time-scale {
		position: relative;
		height: 16px;
		margin-top: 0.25rem;
	}

	.tl-tick {
		position: absolute;
		transform: translateX(-50%);
		font-size: 0.55rem;
		font-family: var(--font-mono);
		color: var(--color-text-muted);
	}
</style>
