import type { AxisCapability } from '@slider/shared';

/** Format an axis value with its unit. e.g. "45.0°", "500.0mm" */
export function formatAxisValue(value: number, axis: AxisCapability): string {
	const formatted = value.toFixed(1);
	return axis.unit === 'deg' ? `${formatted}\u00B0` : `${formatted}${axis.unit}`;
}

/** Compact axis summary for a keyframe. e.g. "Pan 45.0° | Tilt -10.0° | Slide 500.0mm" */
export function formatPositionSummary(
	positions: Record<string, number>,
	axes: AxisCapability[]
): string {
	return axes
		.filter((a) => a.name in positions)
		.map((a) => `${capitalize(a.name)} ${formatAxisValue(positions[a.name], a)}`)
		.join(' | ');
}

function capitalize(s: string): string {
	return s.charAt(0).toUpperCase() + s.slice(1);
}
