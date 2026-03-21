<script lang="ts">
	interface Props {
		tabs: string[];
		active: string;
		onchange: (tab: string) => void;
	}

	let { tabs, active, onchange }: Props = $props();
</script>

<!-- svelte-ignore a11y_no_noninteractive_element_to_interactive_role -->
<nav class="tab-bar" role="tablist">
	{#each tabs as tab}
		<button
			role="tab"
			class="tab"
			class:active={active === tab}
			aria-selected={active === tab}
			onclick={() => onchange(tab)}
		>
			{tab}
		</button>
	{/each}
</nav>

<style>
	.tab-bar {
		display: flex;
		border-bottom: 2px solid var(--color-primary);
	}

	.tab {
		flex: 1;
		padding: 0.625rem 0.5rem;
		font-size: 0.875rem;
		font-weight: 500;
		color: var(--color-text-muted);
		border-bottom: 2px solid transparent;
		margin-bottom: -2px;
		min-width: auto;
		border-radius: 0;
		transition: color 0.15s, border-color 0.15s;
	}

	.tab:hover {
		color: var(--color-text);
	}

	.tab.active {
		color: var(--color-accent);
		border-bottom-color: var(--color-accent);
	}
</style>
