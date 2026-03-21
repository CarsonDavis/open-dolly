import type {
	StatusResponse,
	Capabilities,
	TrajectoryUpload,
	TrajectoryUploadResponse,
	TrajectoryStatus,
	Settings,
	SettingsUpdate,
	SettingsResponse
} from '@opendolly/shared';

export class BoardClient {
	private baseUrl: string;

	constructor(baseUrl: string = '') {
		this.baseUrl = baseUrl;
	}

	private async request<T>(path: string, init?: RequestInit): Promise<T> {
		const res = await fetch(`${this.baseUrl}${path}`, init);
		if (!res.ok) {
			const body = await res.json().catch(() => ({ error: res.statusText }));
			throw new ApiError(res.status, body.error ?? 'Unknown error', body.detail);
		}
		if (res.status === 204) return undefined as T;
		return res.json();
	}

	async getStatus(): Promise<StatusResponse> {
		return this.request('/api/status');
	}

	async getCapabilities(): Promise<Capabilities> {
		return this.request('/api/capabilities');
	}

	async uploadTrajectory(trajectory: TrajectoryUpload): Promise<TrajectoryUploadResponse> {
		return this.request('/api/trajectory', {
			method: 'POST',
			headers: { 'Content-Type': 'application/json' },
			body: JSON.stringify(trajectory)
		});
	}

	async deleteTrajectory(id: string): Promise<void> {
		return this.request(`/api/trajectory/${id}`, { method: 'DELETE' });
	}

	async getTrajectoryStatus(id: string): Promise<TrajectoryStatus> {
		return this.request(`/api/trajectory/${id}/status`);
	}

	async getSettings(): Promise<Settings> {
		return this.request('/api/settings');
	}

	async updateSettings(update: SettingsUpdate): Promise<SettingsResponse> {
		return this.request('/api/settings', {
			method: 'PATCH',
			headers: { 'Content-Type': 'application/json' },
			body: JSON.stringify(update)
		});
	}
}

export class ApiError extends Error {
	constructor(
		public status: number,
		public code: string,
		public detail?: string
	) {
		super(detail ?? code);
		this.name = 'ApiError';
	}
}
