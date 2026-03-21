import { writable } from 'svelte/store';
import type { TrajectoryPoint } from '@opendolly/shared';

export type TrajectoryState = 'empty' | 'computing' | 'computed' | 'uploading' | 'uploaded' | 'error';

export interface TrajectoryStoreValue {
	state: TrajectoryState;
	points: TrajectoryPoint[] | null;
	trajectoryId: string | null;
	error: string | null;
	pointCount: number;
	durationMs: number;
}

const initial: TrajectoryStoreValue = {
	state: 'empty',
	points: null,
	trajectoryId: null,
	error: null,
	pointCount: 0,
	durationMs: 0
};

function createTrajectoryStore() {
	const { subscribe, set, update } = writable<TrajectoryStoreValue>(initial);

	return {
		subscribe,
		setComputing() {
			update((s) => ({ ...s, state: 'computing', error: null }));
		},
		setComputed(points: TrajectoryPoint[], durationMs: number) {
			update((s) => ({
				...s,
				state: 'computed',
				points,
				pointCount: points.length,
				durationMs
			}));
		},
		setUploading() {
			update((s) => ({ ...s, state: 'uploading' }));
		},
		setUploaded(trajectoryId: string) {
			update((s) => ({ ...s, state: 'uploaded', trajectoryId }));
		},
		setError(error: string) {
			update((s) => ({ ...s, state: 'error', error }));
		},
		clear() {
			set(initial);
		}
	};
}

export const trajectoryStore = createTrajectoryStore();
