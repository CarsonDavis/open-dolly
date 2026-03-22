import type {
	WsCommand,
	PositionEvent,
	StateEvent,
	CompleteEvent,
	ErrorEvent
} from '@opendolly/shared';

export type ConnectionState = 'disconnected' | 'connecting' | 'connected' | 'reconnecting';

export interface WebSocketHandlers {
	onPosition: (data: PositionEvent) => void;
	onState: (data: StateEvent) => void;
	onComplete: (data: CompleteEvent) => void;
	onError: (data: ErrorEvent) => void;
	onConnectionChange: (state: ConnectionState) => void;
}

const BACKOFF_SCHEDULE = [1000, 2000, 4000, 8000, 16000, 30000];
const HEARTBEAT_INTERVAL = 5000;
const HEARTBEAT_TIMEOUT = 3000;
const JOG_THROTTLE_MS = 50; // 20 Hz

export class BoardWebSocket {
	private ws: WebSocket | null = null;
	private url: string;
	private handlers: WebSocketHandlers;
	private reconnectAttempt = 0;
	private reconnectTimer: ReturnType<typeof setTimeout> | null = null;
	private heartbeatTimer: ReturnType<typeof setInterval> | null = null;
	private heartbeatTimeoutTimer: ReturnType<typeof setTimeout> | null = null;
	private lastMessageTime = 0;
	private intentionalClose = false;

	// Jog throttling
	private jogTimer: ReturnType<typeof setTimeout> | null = null;
	private pendingJog: Record<string, number> | null = null;

	constructor(url: string, handlers: WebSocketHandlers) {
		this.url = url;
		this.handlers = handlers;
	}

	connect(): void {
		if (this.ws?.readyState === WebSocket.OPEN) return;

		this.intentionalClose = false;
		this.handlers.onConnectionChange('connecting');

		this.ws = new WebSocket(this.url);

		this.ws.onopen = () => {
			this.reconnectAttempt = 0;
			this.handlers.onConnectionChange('connected');
			this.startHeartbeat();
		};

		this.ws.onmessage = (event) => {
			this.lastMessageTime = Date.now();
			this.clearHeartbeatTimeout();

			try {
				const data = JSON.parse(event.data);
				this.dispatch(data);
			} catch {
				// Ignore malformed messages
			}
		};

		this.ws.onclose = () => {
			this.cleanup();
			if (!this.intentionalClose) {
				this.scheduleReconnect();
			} else {
				this.handlers.onConnectionChange('disconnected');
			}
		};

		this.ws.onerror = () => {
			// onclose will fire after onerror
		};
	}

	disconnect(): void {
		this.intentionalClose = true;
		this.clearReconnectTimer();
		this.cleanup();
		this.ws?.close();
		this.ws = null;
		this.handlers.onConnectionChange('disconnected');
	}

	isConnected(): boolean {
		return this.ws?.readyState === WebSocket.OPEN;
	}

	private send(command: WsCommand): void {
		if (this.ws?.readyState === WebSocket.OPEN) {
			this.ws.send(JSON.stringify(command));
		}
	}

	sendJog(axes: Record<string, number>): void {
		this.pendingJog = axes;
		if (!this.jogTimer) {
			this.flushJog();
			this.jogTimer = setTimeout(() => {
				this.jogTimer = null;
				if (this.pendingJog) this.flushJog();
			}, JOG_THROTTLE_MS);
		}
	}

	private flushJog(): void {
		if (!this.pendingJog) return;
		this.send({ cmd: 'jog', ...this.pendingJog } as WsCommand);
		this.pendingJog = null;
	}

	sendMoveTo(positions: Record<string, number>, durationMs: number): void {
		const cmd = { cmd: 'move_to', duration_ms: durationMs, ...positions };
		this.send(cmd as WsCommand);
	}

	sendPlay(trajectoryId: string): void {
		this.send({ cmd: 'play', trajectory_id: trajectoryId });
	}

	sendPause(): void {
		this.send({ cmd: 'pause' });
	}

	sendResume(): void {
		this.send({ cmd: 'resume' });
	}

	sendStop(): void {
		this.send({ cmd: 'stop' });
	}

	sendScrub(timeMs: number): void {
		this.send({ cmd: 'scrub', t: timeMs });
	}

	sendHome(axes: string[]): void {
		this.send({ cmd: 'home', axes });
	}

	private dispatch(data: Record<string, unknown>): void {
		switch (data.evt) {
			case 'position':
				this.handlers.onPosition(data as unknown as PositionEvent);
				break;
			case 'state':
				this.handlers.onState(data as unknown as StateEvent);
				break;
			case 'complete':
				this.handlers.onComplete(data as unknown as CompleteEvent);
				break;
			case 'error':
				this.handlers.onError(data as unknown as ErrorEvent);
				break;
		}
	}

	private scheduleReconnect(): void {
		const delay = BACKOFF_SCHEDULE[Math.min(this.reconnectAttempt, BACKOFF_SCHEDULE.length - 1)];
		this.reconnectAttempt++;
		this.handlers.onConnectionChange('reconnecting');
		this.reconnectTimer = setTimeout(() => {
			this.reconnectTimer = null;
			this.connect();
		}, delay);
	}

	private clearReconnectTimer(): void {
		if (this.reconnectTimer) {
			clearTimeout(this.reconnectTimer);
			this.reconnectTimer = null;
		}
	}

	private startHeartbeat(): void {
		this.stopHeartbeat();
		this.lastMessageTime = Date.now();
		this.heartbeatTimer = setInterval(() => {
			if (Date.now() - this.lastMessageTime > HEARTBEAT_INTERVAL && this.ws?.readyState === WebSocket.OPEN) {
				this.ws.send(JSON.stringify({ cmd: 'ping' }));
				this.heartbeatTimeoutTimer = setTimeout(() => {
					// No pong received — close and reconnect
					this.ws?.close();
				}, HEARTBEAT_TIMEOUT);
			}
		}, HEARTBEAT_INTERVAL);
	}

	private stopHeartbeat(): void {
		if (this.heartbeatTimer) {
			clearInterval(this.heartbeatTimer);
			this.heartbeatTimer = null;
		}
		this.clearHeartbeatTimeout();
	}

	private clearHeartbeatTimeout(): void {
		if (this.heartbeatTimeoutTimer) {
			clearTimeout(this.heartbeatTimeoutTimer);
			this.heartbeatTimeoutTimer = null;
		}
	}

	private cleanup(): void {
		this.stopHeartbeat();
		if (this.jogTimer) {
			clearTimeout(this.jogTimer);
			this.jogTimer = null;
		}
		this.pendingJog = null;
	}
}
