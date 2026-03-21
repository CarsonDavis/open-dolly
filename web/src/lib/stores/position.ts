import { writable } from 'svelte/store';
import type { Position } from '@opendolly/shared';

export const positionStore = writable<Position>({});
