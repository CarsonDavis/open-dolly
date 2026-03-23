import { describe, it, expect } from 'vitest';
import { buildTrajectory, getSequenceDuration, allTransitionsSet } from './trajectory-builder.js';
import type { Sequence } from '$lib/stores/sequence';

function makeSequence(overrides?: Partial<Sequence>): Sequence {
	return {
		keyframes: [
			{ id: 'kf1', label: 'KF1', positions: { slide: 0, pan: 0 }, createdAt: 0 },
			{ id: 'kf2', label: 'KF2', positions: { slide: 500, pan: 45 }, createdAt: 1 }
		],
		transitions: [
			{ id: 'tr1', duration_ms: 5000, curves: {} }
		],
		...overrides
	};
}

const axisTypes = { slide: 'linear' as const, pan: 'rotation' as const };

describe('buildTrajectory', () => {
	it('with 2 keyframes produces valid points', () => {
		const result = buildTrajectory(makeSequence(), axisTypes);
		expect(result.points.length).toBeGreaterThan(0);
		expect(result.points[0].slide).toBe(0);
		expect(result.points[result.points.length - 1].slide).toBe(500);
	});

	it('first and last points have correct timestamps', () => {
		const result = buildTrajectory(makeSequence(), axisTypes);
		expect(result.points[0].t).toBe(0);
		expect(result.points[result.points.length - 1].t).toBe(5000);
	});

	it('timestamps are strictly ascending', () => {
		const result = buildTrajectory(makeSequence(), axisTypes);
		for (let i = 1; i < result.points.length; i++) {
			expect(result.points[i].t).toBeGreaterThan(result.points[i - 1].t);
		}
	});

	it('with per-axis ProgressCurve easing produces valid output', () => {
		const seq = makeSequence();
		seq.transitions[0].curves = {
			slide: { points: [{ t: 0.5, progress: 0.8 }] }
		};
		const result = buildTrajectory(seq, axisTypes);
		expect(result.points.length).toBeGreaterThan(0);
		// At midpoint, slide should be > 250 (linear midpoint) due to ease-out curve
		const midpoint = result.points.find((p) => p.t === 2500);
		expect(midpoint).toBeDefined();
		expect(midpoint!.slide).toBeGreaterThan(250);
	});

	it('concatenates multi-transition sequences correctly', () => {
		const seq: Sequence = {
			keyframes: [
				{ id: 'kf1', label: 'KF1', positions: { slide: 0 }, createdAt: 0 },
				{ id: 'kf2', label: 'KF2', positions: { slide: 500 }, createdAt: 1 },
				{ id: 'kf3', label: 'KF3', positions: { slide: 200 }, createdAt: 2 }
			],
			transitions: [
				{ id: 'tr1', duration_ms: 3000, curves: {} },
				{ id: 'tr2', duration_ms: 2000, curves: {} }
			]
		};
		const result = buildTrajectory(seq, { slide: 'linear' });

		expect(result.points[0].t).toBe(0);
		expect(result.points[0].slide).toBe(0);
		expect(result.points[result.points.length - 1].t).toBe(5000);
		expect(result.points[result.points.length - 1].slide).toBe(200);

		// Midpoint at 3000ms should be at slide=500
		const boundary = result.points.find((p) => p.t === 3000);
		expect(boundary).toBeDefined();
		expect(boundary!.slide).toBe(500);
	});

	it('boundary points between transitions are continuous', () => {
		const seq: Sequence = {
			keyframes: [
				{ id: 'kf1', label: 'KF1', positions: { slide: 0 }, createdAt: 0 },
				{ id: 'kf2', label: 'KF2', positions: { slide: 500 }, createdAt: 1 },
				{ id: 'kf3', label: 'KF3', positions: { slide: 100 }, createdAt: 2 }
			],
			transitions: [
				{ id: 'tr1', duration_ms: 2000, curves: {} },
				{ id: 'tr2', duration_ms: 3000, curves: {} }
			]
		};
		const result = buildTrajectory(seq, { slide: 'linear' });

		// Find points around the boundary (t=2000)
		const beforeBoundary = result.points.filter((p) => p.t < 2000).pop();
		const atBoundary = result.points.find((p) => p.t === 2000);
		const afterBoundary = result.points.find((p) => p.t > 2000);

		expect(atBoundary).toBeDefined();
		expect(atBoundary!.slide).toBeCloseTo(500, 0);

		// No sudden jump
		if (beforeBoundary && afterBoundary) {
			const gap = Math.abs(afterBoundary.slide - atBoundary!.slide);
			expect(gap).toBeLessThan(10); // should be very small
		}
	});

	it('with empty sequence returns empty array', () => {
		const result = buildTrajectory(
			{ keyframes: [], transitions: [] },
			axisTypes
		);
		expect(result.points).toEqual([]);
	});

	it('with single keyframe returns empty array', () => {
		const result = buildTrajectory(
			{
				keyframes: [{ id: 'kf1', label: 'KF1', positions: { slide: 0 }, createdAt: 0 }],
				transitions: []
			},
			axisTypes
		);
		expect(result.points).toEqual([]);
	});
});

describe('allTransitionsSet', () => {
	it('returns false when any duration is 0', () => {
		const seq: Sequence = {
			keyframes: [
				{ id: 'kf1', label: 'KF1', positions: { slide: 0 }, createdAt: 0 },
				{ id: 'kf2', label: 'KF2', positions: { slide: 500 }, createdAt: 1 }
			],
			transitions: [{ id: 'tr1', duration_ms: 0, curves: {} }]
		};
		expect(allTransitionsSet(seq)).toBe(false);
	});

	it('returns true when all durations are > 0', () => {
		expect(allTransitionsSet(makeSequence())).toBe(true);
	});

	it('returns false for empty transitions', () => {
		expect(allTransitionsSet({ keyframes: [], transitions: [] })).toBe(false);
	});
});

describe('getSequenceDuration', () => {
	it('sums all transition durations', () => {
		const seq: Sequence = {
			keyframes: [
				{ id: 'kf1', label: 'KF1', positions: {}, createdAt: 0 },
				{ id: 'kf2', label: 'KF2', positions: {}, createdAt: 1 },
				{ id: 'kf3', label: 'KF3', positions: {}, createdAt: 2 }
			],
			transitions: [
				{ id: 'tr1', duration_ms: 3000, curves: {} },
				{ id: 'tr2', duration_ms: 7000, curves: {} }
			]
		};
		expect(getSequenceDuration(seq)).toBe(10000);
	});
});
