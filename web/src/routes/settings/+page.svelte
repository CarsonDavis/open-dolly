<script lang="ts">
	import { onMount } from 'svelte';
	import { capabilitiesStore } from '$lib/stores/capabilities';
	import { isConnected } from '$lib/stores/connection';
	import { BoardClient } from '$lib/api/client';
	import type { Settings } from '@opendolly/shared';

	let settings: Settings | null = $state(null);
	let loading = $state(true);
	let saving = $state(false);
	let error = $state('');
	let success = $state('');
	let wifiChanged = $state(false);

	// Editable fields
	let deviceName = $state('');
	let apSsid = $state('');
	let apPassword = $state('');
	let telemetryRate = $state(50);
	let jogSensitivity = $state(1.0);
	let homeOnBoot = $state(false);

	// V2 local settings (stored in localStorage, not sent to board)
	const V2_SETTINGS_KEY = 'opendolly-settings-v2';

	interface V2LocalSettings {
		buffersEnabled: boolean;
		bufferPreMs: number;
		bufferPostMs: number;
		movementSpeedPercent: Record<string, number>;
	}

	let buffersEnabled = $state(false);
	let bufferPreMs = $state(1000);
	let bufferPostMs = $state(1000);
	let movementSpeedPercent = $state<Record<string, number>>({});

	function loadV2Settings() {
		if (typeof localStorage === 'undefined') return;
		try {
			const raw = localStorage.getItem(V2_SETTINGS_KEY);
			if (raw) {
				const s: V2LocalSettings = JSON.parse(raw);
				buffersEnabled = s.buffersEnabled ?? false;
				bufferPreMs = s.bufferPreMs ?? 1000;
				bufferPostMs = s.bufferPostMs ?? 1000;
				movementSpeedPercent = s.movementSpeedPercent ?? {};
			}
		} catch { /* ignore */ }
	}

	function saveV2Settings() {
		if (typeof localStorage === 'undefined') return;
		const s: V2LocalSettings = {
			buffersEnabled,
			bufferPreMs,
			bufferPostMs,
			movementSpeedPercent
		};
		localStorage.setItem(V2_SETTINGS_KEY, JSON.stringify(s));
	}

	const baseUrl = import.meta.env.VITE_BOARD_URL ?? '';
	const client = new BoardClient(baseUrl);

	onMount(async () => {
		loadV2Settings();
		try {
			const s = await client.getSettings();
			settings = s;
			deviceName = s.device_name;
			apSsid = s.ap_ssid;
			apPassword = s.ap_password;
			telemetryRate = s.telemetry_rate_hz;
			jogSensitivity = s.jog_sensitivity;
			homeOnBoot = s.home_on_boot;
		} catch (e) {
			error = 'Failed to load settings';
		} finally {
			loading = false;
		}
	});

	function checkWifiChanged() {
		wifiChanged = settings != null && (apSsid !== settings.ap_ssid || apPassword !== settings.ap_password);
	}

	async function save() {
		saving = true;
		error = '';
		success = '';
		try {
			await client.updateSettings({
				device_name: deviceName,
				ap_ssid: apSsid,
				ap_password: apPassword,
				telemetry_rate_hz: telemetryRate,
				jog_sensitivity: jogSensitivity,
				home_on_boot: homeOnBoot
			});
			success = 'Settings saved';
			wifiChanged = false;
		} catch (e) {
			error = e instanceof Error ? e.message : 'Failed to save';
		} finally {
			saving = false;
		}
	}

	async function restart() {
		try {
			await client.updateSettings({ restart: true });
		} catch {
			// Connection will drop on restart — that's expected
		}
	}
</script>

<svelte:head>
	<title>Settings - Slider</title>
</svelte:head>

<div class="settings-page">
	<div class="settings-header">
		<a href="/" class="back-link">&larr; Back</a>
		<h1>Settings</h1>
	</div>

	{#if loading}
		<p class="loading">Loading settings...</p>
	{:else}
		<form onsubmit={(e) => { e.preventDefault(); save(); }}>
			<section class="settings-section">
				<h2>Device</h2>
				<label class="field">
					<span>Device Name</span>
					<input type="text" bind:value={deviceName} />
				</label>
			</section>

			<section class="settings-section">
				<h2>WiFi Access Point</h2>
				<label class="field">
					<span>SSID</span>
					<input type="text" bind:value={apSsid} oninput={checkWifiChanged} />
				</label>
				<label class="field">
					<span>Password</span>
					<input type="text" bind:value={apPassword} oninput={checkWifiChanged} />
				</label>
				{#if wifiChanged}
					<div class="wifi-warning">
						WiFi changes require a device restart to take effect.
						<button type="button" class="restart-btn" onclick={restart}>Restart Now</button>
					</div>
				{/if}
			</section>

			<section class="settings-section">
				<h2>Motion</h2>
				<label class="field">
					<span>Telemetry Rate (Hz)</span>
					<input type="number" bind:value={telemetryRate} min="1" max="100" />
				</label>
				<label class="field">
					<span>Jog Sensitivity</span>
					<input type="range" bind:value={jogSensitivity} min="0.1" max="3.0" step="0.1" class="sensitivity-slider" />
					<span class="range-value">{jogSensitivity.toFixed(1)}</span>
				</label>
				<label class="field checkbox-field">
					<input type="checkbox" bind:checked={homeOnBoot} />
					<span>Home on boot</span>
				</label>
			</section>

			<section class="settings-section">
				<h2>Buffers (Anti-Jitter)</h2>
				<label class="field checkbox-field">
					<input type="checkbox" bind:checked={buffersEnabled} onchange={saveV2Settings} />
					<span>Enable buffers</span>
				</label>
				{#if buffersEnabled}
					<label class="field">
						<span>Pre-buffer (ms)</span>
						<input type="number" bind:value={bufferPreMs} min="100" max="5000" step="100" onchange={saveV2Settings} />
					</label>
					<label class="field">
						<span>Post-buffer (ms)</span>
						<input type="number" bind:value={bufferPostMs} min="100" max="5000" step="100" onchange={saveV2Settings} />
					</label>
				{/if}
			</section>

			{#if $capabilitiesStore}
				<section class="settings-section">
					<h2>Movement Speed</h2>
					{#each $capabilitiesStore.axes as axis}
						<label class="field">
							<span>{axis.name} ({(movementSpeedPercent[axis.name] ?? 25)}%)</span>
							<input
								type="range"
								min="5"
								max="100"
								step="5"
								value={movementSpeedPercent[axis.name] ?? 25}
								oninput={(e) => {
									movementSpeedPercent = { ...movementSpeedPercent, [axis.name]: parseInt((e.target as HTMLInputElement).value) };
									saveV2Settings();
								}}
								class="sensitivity-slider"
							/>
						</label>
					{/each}
				</section>
			{/if}

			{#if $capabilitiesStore}
				<section class="settings-section">
					<h2>Board Info</h2>
					<div class="info-row">
						<span class="info-label">Firmware</span>
						<span class="info-value">{$capabilitiesStore.firmware_version}</span>
					</div>
					<div class="info-row">
						<span class="info-label">Board</span>
						<span class="info-value">{$capabilitiesStore.board}</span>
					</div>
					<div class="info-row">
						<span class="info-label">Axes</span>
						<span class="info-value">{$capabilitiesStore.axes.map(a => a.name).join(', ')}</span>
					</div>
				</section>
			{/if}

			{#if error}
				<div class="message error">{error}</div>
			{/if}
			{#if success}
				<div class="message success">{success}</div>
			{/if}

			<button type="submit" class="save-btn" disabled={saving}>
				{saving ? 'Saving...' : 'Save Settings'}
			</button>
		</form>
	{/if}
</div>

<style>
	.settings-page {
		max-width: 500px;
	}

	.settings-header {
		display: flex;
		align-items: center;
		gap: 1rem;
		margin-bottom: 1.5rem;
	}

	.back-link {
		color: var(--color-text-muted);
		text-decoration: none;
		font-size: 0.875rem;
		min-width: var(--touch-min);
		min-height: var(--touch-min);
		display: flex;
		align-items: center;
	}

	.back-link:hover {
		color: var(--color-text);
	}

	h1 {
		font-size: 1.25rem;
		font-weight: 600;
	}

	.loading {
		color: var(--color-text-muted);
		padding: 2rem 0;
	}

	.settings-section {
		margin-bottom: 1.5rem;
	}

	h2 {
		font-size: 0.8rem;
		text-transform: uppercase;
		letter-spacing: 0.05em;
		color: var(--color-text-muted);
		margin-bottom: 0.75rem;
		border-bottom: 1px solid var(--color-primary);
		padding-bottom: 0.375rem;
	}

	.field {
		display: flex;
		flex-direction: column;
		gap: 0.25rem;
		margin-bottom: 0.75rem;
	}

	.field span {
		font-size: 0.875rem;
	}

	.checkbox-field {
		flex-direction: row;
		align-items: center;
		gap: 0.5rem;
	}

	.checkbox-field input {
		width: 1.25rem;
		height: 1.25rem;
		min-height: auto;
	}

	.sensitivity-slider {
		border: none;
		padding: 0;
	}

	.range-value {
		font-family: var(--font-mono);
		font-size: 0.8rem;
		color: var(--color-text-muted);
	}

	.info-row {
		display: flex;
		justify-content: space-between;
		padding: 0.375rem 0;
		font-size: 0.875rem;
	}

	.info-label {
		color: var(--color-text-muted);
	}

	.info-value {
		font-family: var(--font-mono);
	}

	.wifi-warning {
		background: rgba(255, 152, 0, 0.15);
		color: var(--color-warning);
		padding: 0.75rem;
		border-radius: var(--radius);
		font-size: 0.8rem;
		display: flex;
		align-items: center;
		gap: 0.75rem;
		flex-wrap: wrap;
	}

	.restart-btn {
		background: var(--color-warning);
		color: #000;
		padding: 0.375rem 0.75rem;
		border-radius: var(--radius);
		font-size: 0.8rem;
		font-weight: 500;
		min-height: auto;
		min-width: auto;
	}

	.message {
		padding: 0.5rem 0.75rem;
		border-radius: var(--radius);
		font-size: 0.8rem;
		margin-bottom: 0.75rem;
	}

	.message.error {
		background: rgba(244, 67, 54, 0.1);
		color: var(--color-error);
	}

	.message.success {
		background: rgba(76, 175, 80, 0.1);
		color: var(--color-success);
	}

	.save-btn {
		background: var(--color-accent);
		border-radius: var(--radius);
		padding: 0.75rem;
		font-size: 0.9rem;
		font-weight: 600;
		width: 100%;
	}

	.save-btn:hover:not(:disabled) {
		filter: brightness(1.1);
	}

	.save-btn:disabled {
		opacity: 0.5;
		cursor: default;
	}
</style>
