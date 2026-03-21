<script lang="ts">
	import { connectionStore, isConnected } from '$lib/stores/connection';
	import { stateStore, systemState } from '$lib/stores/state';
	import { capabilitiesStore } from '$lib/stores/capabilities';

	const stateLabels: Record<string, string> = {
		idle: 'Idle',
		moving: 'Moving',
		playing: 'Playing',
		paused: 'Paused',
		homing: 'Homing',
		error: 'Error'
	};

	function connectionLabel(state: string): string {
		switch (state) {
			case 'connected':
				return 'Connected';
			case 'connecting':
				return 'Connecting...';
			case 'reconnecting':
				return 'Reconnecting...';
			default:
				return 'Disconnected';
		}
	}
</script>

<header class="status-bar" aria-live="polite">
	<div class="status-left">
		<span class="connection-indicator" class:connected={$isConnected} class:reconnecting={$connectionStore.state === 'reconnecting'} class:disconnected={$connectionStore.state === 'disconnected'}></span>
		<span class="connection-label">{connectionLabel($connectionStore.state)}</span>
		<span class="state-badge" class:error={$stateStore.state === 'error'} class:playing={$stateStore.state === 'playing'}>
			{stateLabels[$stateStore.state] ?? $stateStore.state}
		</span>
	</div>
	<div class="status-right">
		{#if $stateStore.battery != null}
			<span class="battery">{$stateStore.battery}%</span>
		{/if}
		<a href="/settings" class="settings-link" aria-label="Settings">
			<svg width="20" height="20" viewBox="0 0 20 20" fill="currentColor">
				<path d="M10 13a3 3 0 100-6 3 3 0 000 6z"/>
				<path fill-rule="evenodd" d="M11.49 1.17c-.38-1.56-2.6-1.56-2.98 0a1.53 1.53 0 01-2.29.95c-1.37-.84-2.94.73-2.1 2.1.46.75.06 1.73-.95 2.29-1.56.38-1.56 2.6 0 2.98.75.18 1.19.94.95 2.29.84 1.37-.73 2.94-2.1 2.1a1.53 1.53 0 01-2.29.95c-.38 1.56.82 2.98 2.98 0a1.53 1.53 0 012.29-.95c1.37.84 2.94-.73 2.1-2.1a1.53 1.53 0 01.95-2.29c1.56-.38 1.56-2.6 0-2.98a1.53 1.53 0 01-.95-2.29c.84-1.37-.73-2.94-2.1-2.1a1.53 1.53 0 01-2.29-.95z" clip-rule="evenodd"/>
			</svg>
		</a>
	</div>
</header>

<style>
	.status-bar {
		display: flex;
		align-items: center;
		justify-content: space-between;
		padding: 0.5rem 1rem;
		background: var(--color-surface);
		border-bottom: 1px solid var(--color-primary);
		min-height: var(--touch-min);
		gap: 0.75rem;
	}

	.status-left {
		display: flex;
		align-items: center;
		gap: 0.5rem;
		font-size: 0.875rem;
	}

	.connection-indicator {
		width: 8px;
		height: 8px;
		border-radius: 50%;
		background: var(--color-text-muted);
		flex-shrink: 0;
	}

	.connection-indicator.connected {
		background: var(--color-success);
	}

	.connection-indicator.reconnecting {
		background: var(--color-warning);
		animation: pulse 1.5s ease-in-out infinite;
	}

	.connection-indicator.disconnected {
		background: var(--color-error);
	}

	.connection-label {
		color: var(--color-text-muted);
	}

	.state-badge {
		padding: 0.125rem 0.5rem;
		border-radius: 4px;
		background: var(--color-primary);
		font-size: 0.75rem;
		text-transform: uppercase;
		letter-spacing: 0.05em;
	}

	.state-badge.error {
		background: var(--color-error);
	}

	.state-badge.playing {
		background: var(--color-success);
	}

	.settings-link {
		color: var(--color-text-muted);
		display: flex;
		align-items: center;
		justify-content: center;
		min-width: var(--touch-min);
		min-height: var(--touch-min);
	}

	.settings-link:hover {
		color: var(--color-text);
	}

	.battery {
		font-size: 0.8rem;
		font-family: var(--font-mono);
		color: var(--color-text-muted);
	}

	.status-right {
		display: flex;
		align-items: center;
		gap: 0.25rem;
	}

	@keyframes pulse {
		0%, 100% { opacity: 1; }
		50% { opacity: 0.4; }
	}
</style>
