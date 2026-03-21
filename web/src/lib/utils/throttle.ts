/**
 * Trailing-edge throttle. Guarantees the last call is always executed.
 * Returns a function that accepts args and forwards the most recent call
 * after `intervalMs` has elapsed since the previous invocation.
 */
export function throttle<T extends (...args: unknown[]) => void>(
	fn: T,
	intervalMs: number
): (...args: Parameters<T>) => void {
	let timer: ReturnType<typeof setTimeout> | null = null;
	let lastArgs: Parameters<T> | null = null;
	let lastCallTime = 0;

	return (...args: Parameters<T>) => {
		lastArgs = args;
		const now = Date.now();
		const remaining = intervalMs - (now - lastCallTime);

		if (remaining <= 0) {
			if (timer) {
				clearTimeout(timer);
				timer = null;
			}
			lastCallTime = now;
			fn(...args);
			lastArgs = null;
		} else if (!timer) {
			timer = setTimeout(() => {
				timer = null;
				lastCallTime = Date.now();
				if (lastArgs) {
					fn(...lastArgs);
					lastArgs = null;
				}
			}, remaining);
		}
	};
}
