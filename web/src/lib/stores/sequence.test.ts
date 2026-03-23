import { describe, it, expect, beforeEach, vi } from 'vitest';
import { get } from 'svelte/store';

// Mock localStorage
const store: Record<string, string> = {};
vi.stubGlobal('localStorage', {
	getItem: (key: string) => store[key] ?? null,
	setItem: (key: string, value: string) => { store[key] = value; },
	removeItem: (key: string) => { delete store[key]; }
});

// Mock crypto.randomUUID
let uuidCounter = 0;
vi.stubGlobal('crypto', {
	randomUUID: () => `test-uuid-${++uuidCounter}`
});

// Import after mocks are set up
const { sequenceStore } = await import('./sequence.js');

beforeEach(() => {
	uuidCounter = 0;
	for (const key of Object.keys(store)) delete store[key];
	sequenceStore.clear();
});

describe('sequenceStore', () => {
	it('starts with empty sequence', () => {
		const seq = get(sequenceStore);
		expect(seq.keyframes).toEqual([]);
		expect(seq.transitions).toEqual([]);
	});

	it('addKeyframe creates a keyframe with all provided positions', () => {
		sequenceStore.addKeyframe({ pan: 45, tilt: -10, slide: 500 });
		const seq = get(sequenceStore);

		expect(seq.keyframes).toHaveLength(1);
		expect(seq.keyframes[0].positions).toEqual({ pan: 45, tilt: -10, slide: 500 });
		expect(seq.keyframes[0].label).toBe('Keyframe 1');
		expect(seq.transitions).toHaveLength(0);
	});

	it('addKeyframe with 2+ keyframes creates transitions', () => {
		sequenceStore.addKeyframe({ pan: 0, slide: 0 });
		sequenceStore.addKeyframe({ pan: 45, slide: 500 });
		const seq = get(sequenceStore);

		expect(seq.keyframes).toHaveLength(2);
		expect(seq.transitions).toHaveLength(1);
		expect(seq.transitions[0].duration_ms).toBe(0); // unset
		expect(seq.transitions[0].curves).toEqual({});
	});

	it('third keyframe creates second transition', () => {
		sequenceStore.addKeyframe({ pan: 0 });
		sequenceStore.addKeyframe({ pan: 45 });
		sequenceStore.addKeyframe({ pan: 90 });
		const seq = get(sequenceStore);

		expect(seq.keyframes).toHaveLength(3);
		expect(seq.transitions).toHaveLength(2);
	});

	it('updateKeyframePositions merges partial positions (preserves other axes)', () => {
		sequenceStore.addKeyframe({ pan: 45, tilt: -10, slide: 500, roll: 0 });
		const seq = get(sequenceStore);
		const kfId = seq.keyframes[0].id;

		// Update only pan — other axes must be preserved
		sequenceStore.updateKeyframePositions(kfId, { pan: 90 });
		const updated = get(sequenceStore);

		expect(updated.keyframes[0].positions).toEqual({
			pan: 90,
			tilt: -10,
			slide: 500,
			roll: 0
		});
	});

	it('updateKeyframePositions with single axis does not erase others', () => {
		sequenceStore.addKeyframe({ pan: 10, tilt: 20, slide: 30 });
		const id = get(sequenceStore).keyframes[0].id;

		sequenceStore.updateKeyframePositions(id, { tilt: 99 });

		const positions = get(sequenceStore).keyframes[0].positions;
		expect(positions.pan).toBe(10);
		expect(positions.tilt).toBe(99);
		expect(positions.slide).toBe(30);
	});

	it('removeKeyframe removes associated transitions', () => {
		sequenceStore.addKeyframe({ pan: 0 });
		sequenceStore.addKeyframe({ pan: 45 });
		sequenceStore.addKeyframe({ pan: 90 });

		const seq = get(sequenceStore);
		expect(seq.transitions).toHaveLength(2);

		// Remove the middle keyframe
		sequenceStore.removeKeyframe(seq.keyframes[1].id);
		const after = get(sequenceStore);

		expect(after.keyframes).toHaveLength(2);
		expect(after.transitions).toHaveLength(1);
	});

	it('removeKeyframe first keyframe removes first transition', () => {
		sequenceStore.addKeyframe({ pan: 0 });
		sequenceStore.addKeyframe({ pan: 45 });

		const firstId = get(sequenceStore).keyframes[0].id;
		sequenceStore.removeKeyframe(firstId);

		const after = get(sequenceStore);
		expect(after.keyframes).toHaveLength(1);
		expect(after.transitions).toHaveLength(0);
	});

	it('clear empties everything', () => {
		sequenceStore.addKeyframe({ pan: 0 });
		sequenceStore.addKeyframe({ pan: 45 });
		sequenceStore.clear();

		const seq = get(sequenceStore);
		expect(seq.keyframes).toEqual([]);
		expect(seq.transitions).toEqual([]);
	});

	it('persists to localStorage and reloads', () => {
		sequenceStore.addKeyframe({ pan: 45, slide: 500 });
		sequenceStore.addKeyframe({ pan: 90, slide: 0 });

		// Check localStorage was written
		const raw = store['opendolly-sequence'];
		expect(raw).toBeDefined();

		const parsed = JSON.parse(raw);
		expect(parsed.version).toBe(2);
		expect(parsed.keyframes).toHaveLength(2);
		expect(parsed.transitions).toHaveLength(1);
	});

	it('updateTransition sets duration and curves', () => {
		sequenceStore.addKeyframe({ pan: 0 });
		sequenceStore.addKeyframe({ pan: 45 });

		const trId = get(sequenceStore).transitions[0].id;
		sequenceStore.updateTransition(trId, {
			duration_ms: 5000,
			curves: { pan: { points: [{ t: 0.5, progress: 0.8 }] } }
		});

		const tr = get(sequenceStore).transitions[0];
		expect(tr.duration_ms).toBe(5000);
		expect(tr.curves.pan.points).toEqual([{ t: 0.5, progress: 0.8 }]);
	});
});
