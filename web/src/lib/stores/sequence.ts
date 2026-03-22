import { writable, get } from 'svelte/store';

export interface SequenceKeyframe {
	id: string;
	label: string;
	positions: Record<string, number>;
	createdAt: number;
}

export interface ProgressCurvePoint {
	t: number;
	progress: number;
}

export interface AxisCurve {
	points: ProgressCurvePoint[];
}

export interface BufferConfig {
	enabled: boolean;
	pre_ms: number;
	post_ms: number;
}

export interface SequenceTransition {
	id: string;
	duration_ms: number;
	curves: Record<string, AxisCurve>;
	buffer?: BufferConfig;
}

export interface Sequence {
	keyframes: SequenceKeyframe[];
	/** transitions[i] connects keyframes[i] to keyframes[i+1]. Length = max(0, keyframes.length - 1). */
	transitions: SequenceTransition[];
}

const V2_STORAGE_KEY = 'opendolly-sequence';
const V1_STORAGE_KEY = 'opendolly-keyframes';

interface StoredSequence {
	version: number;
	keyframes: SequenceKeyframe[];
	transitions: SequenceTransition[];
}

function loadSequence(): Sequence {
	if (typeof localStorage === 'undefined') return { keyframes: [], transitions: [] };

	try {
		// Try V2 format first
		const v2Raw = localStorage.getItem(V2_STORAGE_KEY);
		if (v2Raw) {
			const stored: StoredSequence = JSON.parse(v2Raw);
			if (stored.version === 2) {
				return { keyframes: stored.keyframes, transitions: stored.transitions };
			}
		}

		// Try V1 format and migrate
		const v1Raw = localStorage.getItem(V1_STORAGE_KEY);
		if (v1Raw) {
			const v1Keyframes: SequenceKeyframe[] = JSON.parse(v1Raw);
			const transitions: SequenceTransition[] = [];
			for (let i = 0; i < v1Keyframes.length - 1; i++) {
				transitions.push({
					id: crypto.randomUUID(),
					duration_ms: 0,
					curves: {}
				});
			}
			const sequence = { keyframes: v1Keyframes, transitions };
			// Persist as V2 and remove V1
			persistSequence(sequence);
			localStorage.removeItem(V1_STORAGE_KEY);
			return sequence;
		}
	} catch {
		// Corrupted data — start fresh
	}

	return { keyframes: [], transitions: [] };
}

function persistSequence(sequence: Sequence): void {
	if (typeof localStorage !== 'undefined') {
		const stored: StoredSequence = {
			version: 2,
			keyframes: sequence.keyframes,
			transitions: sequence.transitions
		};
		localStorage.setItem(V2_STORAGE_KEY, JSON.stringify(stored));
	}
}

function createSequenceStore() {
	const { subscribe, set, update } = writable<Sequence>(loadSequence());

	function save(updater: (seq: Sequence) => Sequence): void {
		update((seq) => {
			const next = updater(seq);
			persistSequence(next);
			return next;
		});
	}

	return {
		subscribe,

		addKeyframe(positions: Record<string, number>) {
			const seq = get({ subscribe });
			const kf: SequenceKeyframe = {
				id: crypto.randomUUID(),
				label: `Keyframe ${seq.keyframes.length + 1}`,
				positions: { ...positions },
				createdAt: Date.now()
			};

			save((s) => {
				const keyframes = [...s.keyframes, kf];
				const transitions = [...s.transitions];
				// If this creates a pair, add a transition
				if (keyframes.length >= 2) {
					transitions.push({
						id: crypto.randomUUID(),
						duration_ms: 0,
						curves: {}
					});
				}
				return { keyframes, transitions };
			});

			return kf.id;
		},

		removeKeyframe(id: string) {
			save((s) => {
				const idx = s.keyframes.findIndex((kf) => kf.id === id);
				if (idx === -1) return s;

				const keyframes = s.keyframes.filter((kf) => kf.id !== id);
				const transitions = [...s.transitions];

				if (s.keyframes.length <= 1) {
					// Removing the only keyframe — no transitions to adjust
					return { keyframes, transitions: [] };
				}

				if (idx === 0) {
					// Removing first keyframe — remove first transition
					transitions.splice(0, 1);
				} else if (idx === s.keyframes.length - 1) {
					// Removing last keyframe — remove last transition
					transitions.splice(transitions.length - 1, 1);
				} else {
					// Removing middle keyframe — merge two transitions into one
					// Keep the first transition's data, remove the second
					transitions.splice(idx, 1);
				}

				return { keyframes, transitions };
			});
		},

		reorderKeyframe(fromIndex: number, toIndex: number) {
			save((s) => {
				if (fromIndex < 0 || fromIndex >= s.keyframes.length) return s;
				if (toIndex < 0 || toIndex >= s.keyframes.length) return s;
				if (fromIndex === toIndex) return s;

				const keyframes = [...s.keyframes];
				const [item] = keyframes.splice(fromIndex, 1);
				keyframes.splice(toIndex, 0, item);

				// Rebuild transitions — keep existing curve/duration data where possible
				// For simplicity, reset transitions on reorder (curves are position-dependent)
				const transitions: SequenceTransition[] = [];
				for (let i = 0; i < keyframes.length - 1; i++) {
					transitions.push({
						id: crypto.randomUUID(),
						duration_ms: 0,
						curves: {}
					});
				}

				return { keyframes, transitions };
			});
		},

		updateKeyframePositions(id: string, positions: Record<string, number>) {
			save((s) => ({
				...s,
				keyframes: s.keyframes.map((kf) =>
					kf.id === id ? { ...kf, positions: { ...positions } } : kf
				)
			}));
		},

		updateKeyframeLabel(id: string, label: string) {
			save((s) => ({
				...s,
				keyframes: s.keyframes.map((kf) =>
					kf.id === id ? { ...kf, label } : kf
				)
			}));
		},

		updateTransition(id: string, updates: Partial<Pick<SequenceTransition, 'duration_ms' | 'curves'>>) {
			save((s) => ({
				...s,
				transitions: s.transitions.map((tr) =>
					tr.id === id ? { ...tr, ...updates } : tr
				)
			}));
		},

		clear() {
			save(() => ({ keyframes: [], transitions: [] }));
		}
	};
}

export const sequenceStore = createSequenceStore();

/** Selected keyframe ID — when set, control panel edits overwrite this keyframe. */
export const selectedKeyframeId = writable<string | null>(null);
