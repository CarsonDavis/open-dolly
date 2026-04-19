<script lang="ts">
	import { connectionStore, isConnected } from '$lib/stores/connection';
	import { hasError } from '$lib/stores/state';
	import { axisNames } from '$lib/stores/capabilities';

	function ws() {
		return connectionStore.getWebSocket();
	}

	function handleEStop() {
		ws()?.sendStop();
	}

	function handleHome() {
		ws()?.sendHome($axisNames);
	}

	function handleClearError() {
		ws()?.sendClearError();
	}

	function handleDisable() {
		ws()?.sendDisable();
	}

	function handleRestart() {
		if (!confirm('Reboot the controller? The WS connection will drop for a few seconds.')) {
			return;
		}
		ws()?.sendRestart();
	}
</script>

<div class="bringup-toolbar" role="toolbar" aria-label="Bring-up controls">
	<button
		class="btn estop"
		onclick={handleEStop}
		disabled={!$isConnected}
		title="Immediately stop all motion"
	>
		⛔ E-STOP
	</button>

	<button
		class="btn home"
		onclick={handleHome}
		disabled={!$isConnected || $axisNames.length === 0}
		title="Run homing sequence on all axes"
	>
		⌂ Home
	</button>

	{#if $hasError}
		<button
			class="btn clear"
			onclick={handleClearError}
			disabled={!$isConnected}
			title="Clear latched ERROR state back to IDLE"
		>
			✓ Clear Error
		</button>
	{/if}

	<button
		class="btn disable"
		onclick={handleDisable}
		disabled={!$isConnected}
		title="De-energize motor coils (useful between tests to cool the driver)"
	>
		⏻ Disable
	</button>

	<button
		class="btn restart"
		onclick={handleRestart}
		disabled={!$isConnected}
		title="Reboot the controller"
	>
		↻ Restart
	</button>
</div>

<style>
	.bringup-toolbar {
		display: flex;
		align-items: center;
		gap: 0.5rem;
		padding: 0.5rem 1rem;
		background: var(--color-surface);
		border-bottom: 1px solid var(--color-primary);
		flex-wrap: wrap;
	}

	.btn {
		font-size: 0.8rem;
		padding: 0.5rem 0.875rem;
		min-height: var(--touch-min);
		border-radius: var(--radius);
		background: var(--color-primary);
		color: var(--color-text);
		font-weight: 500;
		cursor: pointer;
		border: 1px solid transparent;
		white-space: nowrap;
	}

	.btn:hover:not(:disabled) {
		background: var(--color-accent);
	}

	.btn:disabled {
		opacity: 0.35;
		cursor: not-allowed;
	}

	.btn.estop {
		background: var(--color-error);
		font-size: 0.95rem;
		font-weight: 700;
		letter-spacing: 0.05em;
		padding: 0.5rem 1.25rem;
	}

	.btn.estop:hover:not(:disabled) {
		background: #d62828;
	}

	.btn.home {
		background: var(--color-success, #2a9d8f);
	}

	.btn.clear {
		background: var(--color-warning, #e9c46a);
		color: #1a1a1a;
		animation: pulse-clear 1.4s ease-in-out infinite;
	}

	.btn.disable,
	.btn.restart {
		background: var(--color-surface);
		border-color: var(--color-primary);
		color: var(--color-text-muted);
	}

	.btn.disable:hover:not(:disabled),
	.btn.restart:hover:not(:disabled) {
		color: var(--color-text);
		background: var(--color-primary);
	}

	@keyframes pulse-clear {
		0%, 100% { box-shadow: 0 0 0 0 rgba(233, 196, 106, 0.5); }
		50% { box-shadow: 0 0 0 6px rgba(233, 196, 106, 0); }
	}
</style>
