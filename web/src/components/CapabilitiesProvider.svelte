<script lang="ts">
	import { onMount, onDestroy, setContext } from 'svelte';
	import { connectionStore, isConnected } from '$lib/stores/connection';
	import { capabilitiesStore } from '$lib/stores/capabilities';
	import { stateStore } from '$lib/stores/state';
	import { positionStore } from '$lib/stores/position';
	import { BoardClient } from '$lib/api/client';
	import type { Snippet } from 'svelte';

	interface Props {
		children: Snippet;
	}

	let { children }: Props = $props();

	const baseUrl = import.meta.env.VITE_BOARD_URL ?? '';
	const wsUrl = import.meta.env.VITE_BOARD_WS_URL ?? `${location.protocol === 'https:' ? 'wss:' : 'ws:'}//${location.host}/ws`;

	let unsubscribe: (() => void) | null = null;
	let statusPollTimer: ReturnType<typeof setInterval> | null = null;
	const client = new BoardClient(baseUrl);

	async function fetchCapabilities(retries = 3): Promise<void> {
		for (let i = 0; i < retries; i++) {
			try {
				const caps = await client.getCapabilities();
				capabilitiesStore.set(caps);
				return;
			} catch {
				if (i < retries - 1) {
					await new Promise((r) => setTimeout(r, 1000 * (i + 1)));
				}
			}
		}
		capabilitiesStore.set(null);
	}

	async function pollStatus(): Promise<void> {
		try {
			const status = await client.getStatus();
			positionStore.set(status.position);
			stateStore.update((s) => ({
				...s,
				battery: status.battery,
				trajectoryId: status.trajectory_loaded ?? s.trajectoryId
			}));
		} catch {
			// Ignore — WebSocket events are the primary source
		}
	}

	function startStatusPoll(): void {
		stopStatusPoll();
		pollStatus();
		statusPollTimer = setInterval(pollStatus, 5000);
	}

	function stopStatusPoll(): void {
		if (statusPollTimer) {
			clearInterval(statusPollTimer);
			statusPollTimer = null;
		}
	}

	onMount(() => {
		connectionStore.connect(wsUrl);

		unsubscribe = isConnected.subscribe((connected) => {
			if (connected) {
				fetchCapabilities();
				startStatusPoll();
			} else {
				capabilitiesStore.set(null);
				stopStatusPoll();
			}
		});
	});

	onDestroy(() => {
		unsubscribe?.();
		stopStatusPoll();
		connectionStore.disconnect();
	});

	setContext('capabilities', capabilitiesStore);
</script>

{@render children()}
