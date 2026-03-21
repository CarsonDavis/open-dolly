import { writable, derived } from 'svelte/store';
import type { SystemState } from '@opendolly/shared';

export interface StateStoreValue {
	state: SystemState;
	error: { code: string; axis?: string; detail: string } | null;
	trajectoryId: string | null;
	playbackProgress: number | null;
	playbackElapsedMs: number | null;
	battery: number | null;
}

const initial: StateStoreValue = {
	state: 'idle',
	error: null,
	trajectoryId: null,
	playbackProgress: null,
	playbackElapsedMs: null,
	battery: null
};

export const stateStore = writable<StateStoreValue>(initial);

export const systemState = derived(stateStore, ($s) => $s.state);
export const isIdle = derived(stateStore, ($s) => $s.state === 'idle');
export const isPlaying = derived(stateStore, ($s) => $s.state === 'playing');
export const hasError = derived(stateStore, ($s) => $s.state === 'error');
