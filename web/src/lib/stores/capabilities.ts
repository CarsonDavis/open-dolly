import { writable, derived } from 'svelte/store';
import type { Capabilities, AxisCapability } from '@slider/shared';

export const capabilitiesStore = writable<Capabilities | null>(null);

export const axisNames = derived(capabilitiesStore, ($c) => $c?.axes.map((a) => a.name) ?? []);

export const axisMap = derived(
	capabilitiesStore,
	($c) =>
		Object.fromEntries($c?.axes.map((a) => [a.name, a]) ?? []) as Record<string, AxisCapability>
);
