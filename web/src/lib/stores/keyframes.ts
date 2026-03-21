import { writable, get } from 'svelte/store';

export interface Keyframe {
	id: string;
	label: string;
	positions: Record<string, number>;
	createdAt: number;
}

const STORAGE_KEY = 'opendolly-keyframes';

function loadKeyframes(): Keyframe[] {
	if (typeof localStorage === 'undefined') return [];
	try {
		const raw = localStorage.getItem(STORAGE_KEY);
		return raw ? JSON.parse(raw) : [];
	} catch {
		return [];
	}
}

function persist(keyframes: Keyframe[]): void {
	if (typeof localStorage !== 'undefined') {
		localStorage.setItem(STORAGE_KEY, JSON.stringify(keyframes));
	}
}

function createKeyframeStore() {
	const { subscribe, set, update } = writable<Keyframe[]>(loadKeyframes());

	function save(updater: (kfs: Keyframe[]) => Keyframe[]): void {
		update((kfs) => {
			const next = updater(kfs);
			persist(next);
			return next;
		});
	}

	return {
		subscribe,
		add(positions: Record<string, number>) {
			const kfs = get({ subscribe });
			const kf: Keyframe = {
				id: crypto.randomUUID(),
				label: `Keyframe ${kfs.length + 1}`,
				positions: { ...positions },
				createdAt: Date.now()
			};
			save((list) => [...list, kf]);
		},
		remove(id: string) {
			save((list) => list.filter((kf) => kf.id !== id));
		},
		reorder(fromIndex: number, toIndex: number) {
			save((list) => {
				const next = [...list];
				const [item] = next.splice(fromIndex, 1);
				next.splice(toIndex, 0, item);
				return next;
			});
		},
		updateLabel(id: string, label: string) {
			save((list) => list.map((kf) => (kf.id === id ? { ...kf, label } : kf)));
		},
		clear() {
			save(() => []);
		}
	};
}

export const keyframeStore = createKeyframeStore();
