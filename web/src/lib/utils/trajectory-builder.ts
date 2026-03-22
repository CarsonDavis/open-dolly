import type { TrajectoryPoint, AxisCapability } from '@opendolly/shared';
import { generateTrajectory, type AxisInterpolation } from '@opendolly/motion-math';
import type { Sequence, SequenceTransition } from '$lib/stores/sequence';

export interface BufferSettings {
	enabled: boolean;
	default_pre_ms: number;
	default_post_ms: number;
}

export interface BufferWarning {
	axisName: string;
	keyframeLabel: string;
	message: string;
}

/**
 * Build a complete trajectory from a V2 Sequence.
 * Calls generateTrajectory once per transition, then concatenates the results.
 * Optionally inserts buffer hold periods at keyframe boundaries.
 */
export function buildTrajectory(
	sequence: Sequence,
	axisTypes: Record<string, AxisInterpolation>,
	bufferSettings?: BufferSettings,
	axisCapabilities?: AxisCapability[]
): { points: TrajectoryPoint[]; warnings: BufferWarning[] } {
	if (sequence.keyframes.length < 2 || sequence.transitions.length === 0) {
		return { points: [], warnings: [] };
	}

	const allPoints: TrajectoryPoint[] = [];
	const warnings: BufferWarning[] = [];
	let cumulativeTime = 0;

	for (let i = 0; i < sequence.transitions.length; i++) {
		const kfA = sequence.keyframes[i];
		const kfB = sequence.keyframes[i + 1];
		const transition = sequence.transitions[i];

		if (transition.duration_ms <= 0) continue;

		// Resolve buffer config for this transition
		const bufferEnabled = transition.buffer?.enabled ?? bufferSettings?.enabled ?? false;
		const preMs = bufferEnabled ? (transition.buffer?.pre_ms ?? bufferSettings?.default_pre_ms ?? 1000) : 0;
		const postMs = bufferEnabled ? (transition.buffer?.post_ms ?? bufferSettings?.default_post_ms ?? 1000) : 0;

		// Pre-buffer: hold at start position
		if (preMs > 0 && i === 0) {
			// Only add pre-buffer for the first transition (or if previous had no post-buffer)
			checkClearance(kfA.positions, kfA.label, axisCapabilities, warnings);
			appendHold(allPoints, kfA.positions, Object.keys(axisTypes), preMs, cumulativeTime);
			cumulativeTime += preMs;
		}

		// Motion segment
		const segment = generateTrajectory({
			keyframes: [
				{ t: 0, positions: kfA.positions },
				{ t: transition.duration_ms, positions: kfB.positions }
			],
			axisTypes,
			axes: buildAxisConfigs(transition),
			sample_interval_ms: 10,
			loop: false
		});

		const startIdx = allPoints.length > 0 ? 1 : 0;
		for (let j = startIdx; j < segment.points.length; j++) {
			const point = { ...segment.points[j] };
			point.t = segment.points[j].t + cumulativeTime;
			allPoints.push(point);
		}
		cumulativeTime += transition.duration_ms;

		// Post-buffer: hold at end position
		if (postMs > 0) {
			checkClearance(kfB.positions, kfB.label, axisCapabilities, warnings);

			// If there's a next transition with a pre-buffer, merge them
			const nextHasPre = i < sequence.transitions.length - 1 && bufferEnabled;
			const nextPreMs = nextHasPre ? (sequence.transitions[i + 1].buffer?.pre_ms ?? bufferSettings?.default_pre_ms ?? 1000) : 0;
			const holdMs = nextHasPre ? Math.max(postMs, nextPreMs) : postMs;

			appendHold(allPoints, kfB.positions, Object.keys(axisTypes), holdMs, cumulativeTime);
			cumulativeTime += holdMs;
		}
	}

	return { points: allPoints, warnings };
}

/**
 * Append hold points (stationary) to the trajectory.
 */
function appendHold(
	allPoints: TrajectoryPoint[],
	positions: Record<string, number>,
	axisNames: string[],
	holdMs: number,
	startTime: number
): void {
	const interval = 10;
	const numPoints = Math.floor(holdMs / interval) + 1;
	const startIdx = allPoints.length > 0 ? 1 : 0; // skip first if it would duplicate

	for (let i = startIdx; i < numPoints; i++) {
		const t = startTime + i * interval;
		const point: TrajectoryPoint = { t };
		for (const axis of axisNames) {
			point[axis] = Math.round((positions[axis] ?? 0) * 10) / 10;
		}
		allPoints.push(point);
	}
}

/**
 * Check if a keyframe position is near axis limits (warning for buffers).
 */
function checkClearance(
	positions: Record<string, number>,
	keyframeLabel: string,
	axisCapabilities: AxisCapability[] | undefined,
	warnings: BufferWarning[]
): void {
	if (!axisCapabilities) return;

	for (const axis of axisCapabilities) {
		const pos = positions[axis.name];
		if (pos === undefined) continue;

		const range = axis.max - axis.min;
		const margin = range * 0.05; // 5% of range as minimum clearance

		if (pos - axis.min < margin) {
			warnings.push({
				axisName: axis.name,
				keyframeLabel,
				message: `${axis.name} at ${keyframeLabel}: only ${(pos - axis.min).toFixed(1)}${axis.unit} from minimum`
			});
		}
		if (axis.max - pos < margin) {
			warnings.push({
				axisName: axis.name,
				keyframeLabel,
				message: `${axis.name} at ${keyframeLabel}: only ${(axis.max - pos).toFixed(1)}${axis.unit} from maximum`
			});
		}
	}
}

/**
 * Get the total duration of all transitions in a sequence (excluding buffers).
 */
export function getSequenceDuration(sequence: Sequence): number {
	return sequence.transitions.reduce((sum, tr) => sum + tr.duration_ms, 0);
}

/**
 * Check if all transitions have valid durations (> 0).
 */
export function allTransitionsSet(sequence: Sequence): boolean {
	return sequence.transitions.length > 0 &&
		sequence.transitions.every((tr) => tr.duration_ms > 0);
}

/**
 * Convert a SequenceTransition's curves to motion-math AxisConfig format.
 */
function buildAxisConfigs(
	transition: SequenceTransition
): Record<string, { easing: { points: Array<{ t: number; progress: number }> } }> {
	const configs: Record<string, { easing: { points: Array<{ t: number; progress: number }> } }> = {};

	for (const [axis, curve] of Object.entries(transition.curves)) {
		if (curve.points.length > 0) {
			configs[axis] = { easing: { points: curve.points } };
		}
	}

	return configs;
}
