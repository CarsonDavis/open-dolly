import { writable } from 'svelte/store';
import type { Position } from '@slider/shared';

export const positionStore = writable<Position>({});
