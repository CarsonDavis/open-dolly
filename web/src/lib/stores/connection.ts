import { writable, derived, get } from 'svelte/store';
import { BoardWebSocket, type ConnectionState, type WebSocketHandlers } from '$lib/api/websocket';
import { positionStore } from './position';
import { stateStore } from './state';
import { trajectoryStore } from './trajectory';

interface ConnectionStoreValue {
	state: ConnectionState;
	url: string | null;
	lastConnected: number | null;
	error: string | null;
	reconnectAttempt: number;
}

const initial: ConnectionStoreValue = {
	state: 'disconnected',
	url: null,
	lastConnected: null,
	error: null,
	reconnectAttempt: 0
};

function createConnectionStore() {
	const { subscribe, set, update } = writable<ConnectionStoreValue>(initial);
	let ws: BoardWebSocket | null = null;

	const handlers: WebSocketHandlers = {
		onPosition(data) {
			const { evt, t, ...axes } = data;
			const positions: Record<string, number> = {};
			for (const [key, value] of Object.entries(axes)) {
				if (typeof value === 'number') positions[key] = value;
			}
			const axisCount = Object.keys(positions).length;
			if (axisCount < 4) {
				console.warn(`[onPosition] PARTIAL position event! Only ${axisCount} axes:`, positions, 'raw data:', data);
			}
			positionStore.set(positions);

			// Update playback progress if playing
			const currentState = get(stateStore);
			if (currentState.state === 'playing' && typeof t === 'number') {
				const traj = get(trajectoryStore);
				if (traj.durationMs > 0) {
					stateStore.update((s) => ({
						...s,
						playbackElapsedMs: t,
						playbackProgress: Math.min(t / traj.durationMs, 1)
					}));
				}
			}
		},
		onState(data) {
			stateStore.update((s) => ({ ...s, state: data.state }));
		},
		onComplete(data) {
			stateStore.update((s) => ({
				...s,
				state: 'idle',
				playbackProgress: null,
				playbackElapsedMs: null
			}));
		},
		onError(data) {
			stateStore.update((s) => ({
				...s,
				state: 'error',
				error: { code: data.code, axis: data.axis, detail: data.detail }
			}));
		},
		onConnectionChange(state) {
			update((s) => ({
				...s,
				state,
				lastConnected: state === 'connected' ? Date.now() : s.lastConnected,
				reconnectAttempt: state === 'reconnecting' ? s.reconnectAttempt + 1 : 0,
				error: state === 'connected' ? null : s.error
			}));
		}
	};

	return {
		subscribe,
		connect(url: string) {
			ws?.disconnect();
			ws = new BoardWebSocket(url, handlers);
			update((s) => ({ ...s, url, error: null }));
			ws.connect();
		},
		disconnect() {
			ws?.disconnect();
			ws = null;
			set(initial);
		},
		getWebSocket(): BoardWebSocket | null {
			return ws;
		}
	};
}

export const connectionStore = createConnectionStore();
export const isConnected = derived(connectionStore, ($c) => $c.state === 'connected');
