<script lang="ts">
	import { capabilitiesStore } from '$lib/stores/capabilities';
	import { isConnected } from '$lib/stores/connection';
	import JogControl from '../components/JogControl.svelte';
	import KeyframeList from '../components/KeyframeList.svelte';
	import PlaybackControls from '../components/PlaybackControls.svelte';
	import TabBar from '../components/TabBar.svelte';

	const tabs = ['Jog', 'Keyframes', 'Play'];
	let activeTab = $state('Jog');

	let isMobile = $state(typeof window !== 'undefined' && window.innerWidth < 640);

	function checkMobile() {
		isMobile = window.innerWidth < 640;
	}

	$effect(() => {
		checkMobile();
	});
</script>

<svelte:window onresize={checkMobile} />
<svelte:head>
	<title>Slider</title>
</svelte:head>

{#if !$capabilitiesStore}
	<div class="connecting">
		{#if $isConnected}
			<p>Loading capabilities...</p>
		{:else}
			<p>Connecting to board...</p>
			<p class="hint">Make sure you're connected to the slider's WiFi network.</p>
		{/if}
	</div>
{:else}
	{#if isMobile}
		<TabBar {tabs} active={activeTab} onchange={(t) => activeTab = t} />

		<div class="tab-content">
			{#if activeTab === 'Jog'}
				<JogControl axes={$capabilitiesStore.axes} />
			{:else if activeTab === 'Keyframes'}
				<KeyframeList axes={$capabilitiesStore.axes} />
			{:else if activeTab === 'Play'}
				<PlaybackControls />
			{/if}
		</div>
	{:else}
		<div class="desktop-layout">
			<div class="left-panel">
				<JogControl axes={$capabilitiesStore.axes} />
			</div>
			<div class="right-panel">
				<KeyframeList axes={$capabilitiesStore.axes} />
			</div>
		</div>
		<div class="bottom-panel">
			<PlaybackControls />
		</div>
	{/if}
{/if}

<style>
	.connecting {
		display: flex;
		flex-direction: column;
		align-items: center;
		justify-content: center;
		padding: 4rem 1rem;
		text-align: center;
		color: var(--color-text-muted);
	}

	.hint {
		font-size: 0.8rem;
		margin-top: 0.5rem;
	}

	.tab-content {
		padding: 1rem 0;
	}

	.desktop-layout {
		display: grid;
		grid-template-columns: 1fr 1fr;
		gap: 1.5rem;
		margin-bottom: 1.5rem;
	}

	.left-panel,
	.right-panel {
		min-width: 0;
	}

	.bottom-panel {
		max-width: 600px;
	}
</style>
