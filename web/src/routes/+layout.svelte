<script lang="ts">
	import '../app.css';
	import BringupToolbar from '../components/BringupToolbar.svelte';
	import CapabilitiesProvider from '../components/CapabilitiesProvider.svelte';
	import StatusBar from '../components/StatusBar.svelte';
	import type { Snippet } from 'svelte';

	interface Props {
		children: Snippet;
	}

	let { children }: Props = $props();
</script>

<CapabilitiesProvider>
	<div class="app-shell">
		<StatusBar />
		<BringupToolbar />
		<main class="app-content">
			{@render children()}
		</main>
	</div>
</CapabilitiesProvider>

<style>
	.app-shell {
		display: flex;
		flex-direction: column;
		min-height: 100vh;
		max-width: 1200px;
		margin: 0 auto;
	}

	.app-content {
		flex: 1;
		padding: 1rem;
	}
</style>
