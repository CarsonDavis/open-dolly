<script lang="ts">
	import { capabilitiesStore } from '$lib/stores/capabilities';
	import { isConnected } from '$lib/stores/connection';
	import ControlPanel from '../components/ControlPanel.svelte';
	import KeyframeList from '../components/KeyframeList.svelte';
	import PlaybackControls from '../components/PlaybackControls.svelte';
	import TimelineView from '../components/TimelineView.svelte';

	let isMobile = $state(typeof window !== 'undefined' && window.innerWidth < 640);
	let isDesktop = $state(typeof window !== 'undefined' && window.innerWidth >= 1024);

	function checkBreakpoints() {
		isMobile = window.innerWidth < 640;
		isDesktop = window.innerWidth >= 1024;
	}

	$effect(() => {
		checkBreakpoints();
	});
</script>

<svelte:window onresize={checkBreakpoints} />
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
		<!-- Phone portrait: keyframes top, control panel bottom -->
		<div class="portrait-layout">
			<div class="keyframe-area">
				<KeyframeList axes={$capabilitiesStore.axes} />
			</div>
			<div class="control-area">
				<ControlPanel axes={$capabilitiesStore.axes} />
			</div>
			<div class="transport-area">
				<PlaybackControls />
			</div>
		</div>
	{:else}
		<!-- Landscape / Desktop: split layout -->
		<div class="landscape-layout">
			<div class="left-panel">
				<ControlPanel axes={$capabilitiesStore.axes} />
			</div>
			<div class="right-panel">
				<KeyframeList axes={$capabilitiesStore.axes} />
			</div>
		</div>
		<div class="bottom-panel">
			<PlaybackControls />
		</div>
		{#if isDesktop}
			<div class="timeline-panel">
				<TimelineView axes={$capabilitiesStore.axes} />
			</div>
		{/if}
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

	/* Portrait phone layout */
	.portrait-layout {
		display: flex;
		flex-direction: column;
		min-height: calc(100vh - 3rem); /* account for status bar */
	}

	.keyframe-area {
		flex: 1;
		overflow-y: auto;
		padding: 0.5rem 0;
	}

	.control-area {
		flex-shrink: 0;
	}

	.transport-area {
		flex-shrink: 0;
		padding-top: 0.5rem;
	}

	/* Landscape / Desktop layout */
	.landscape-layout {
		display: grid;
		grid-template-columns: 2fr 3fr;
		gap: 1rem;
		margin-bottom: 1rem;
	}

	.left-panel,
	.right-panel {
		min-width: 0;
		max-height: calc(100vh - 8rem);
		overflow-y: auto;
	}

	.bottom-panel {
		max-width: 600px;
	}

	.timeline-panel {
		margin-top: 1rem;
	}
</style>
