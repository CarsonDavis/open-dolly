<script lang="ts">
	import { createProgressCurveFunction, type ProgressCurvePoint } from '@opendolly/motion-math';

	interface Props {
		axisName: string;
		points: ProgressCurvePoint[];
		durationSec: number;
		linked?: boolean;
		compact?: boolean;
		onpointschange?: (points: ProgressCurvePoint[]) => void;
		onlinkchange?: (linked: boolean) => void;
	}

	let {
		axisName,
		points,
		durationSec,
		linked = false,
		compact = false,
		onpointschange,
		onlinkchange
	}: Props = $props();

	// SVG dimensions
	const W = 300;
	let H = $derived(compact ? 60 : 120);
	const PAD = { top: 4, right: 4, bottom: 16, left: 24 };
	let plotW = $derived(W - PAD.left - PAD.right);
	let plotH = $derived(H - PAD.top - PAD.bottom);

	let draggingIdx = $state<number | null>(null);
	let svgElement: SVGSVGElement | undefined = $state();

	let displayName = $derived(axisName.charAt(0).toUpperCase() + axisName.slice(1));

	// Build curve path
	let curvePath = $derived.by(() => {
		const fn = createProgressCurveFunction(points);
		const samples = compact ? 50 : 100;
		const parts: string[] = [];
		for (let i = 0; i <= samples; i++) {
			const t = i / samples;
			const p = fn(t);
			const x = PAD.left + t * plotW;
			const y = PAD.top + (1 - p) * plotH;
			parts.push(`${i === 0 ? 'M' : 'L'}${x.toFixed(1)},${y.toFixed(1)}`);
		}
		return parts.join(' ');
	});

	// Point positions in SVG coords
	let pointPositions = $derived(
		points.map((pt) => ({
			x: PAD.left + pt.t * plotW,
			y: PAD.top + (1 - pt.progress) * plotH
		}))
	);

	function svgToNormalized(clientX: number, clientY: number): { t: number; progress: number } | null {
		if (!svgElement) return null;
		const rect = svgElement.getBoundingClientRect();
		const svgX = (clientX - rect.left) * (W / rect.width);
		const svgY = (clientY - rect.top) * (H / rect.height);
		const t = Math.max(0.01, Math.min(0.99, (svgX - PAD.left) / plotW));
		const progress = Math.max(0, Math.min(1, 1 - (svgY - PAD.top) / plotH));
		return { t: Math.round(t * 1000) / 1000, progress: Math.round(progress * 1000) / 1000 };
	}

	function handleSvgClick(e: MouseEvent) {
		if (compact || draggingIdx !== null) return;
		const coord = svgToNormalized(e.clientX, e.clientY);
		if (!coord) return;

		// Add point sorted by t
		const newPoints = [...points, { t: coord.t, progress: coord.progress }]
			.sort((a, b) => a.t - b.t);
		onpointschange?.(newPoints);
	}

	function handlePointDown(idx: number, e: PointerEvent) {
		if (compact) return;
		e.stopPropagation();
		draggingIdx = idx;
		(e.target as SVGElement).setPointerCapture(e.pointerId);
	}

	function handlePointMove(e: PointerEvent) {
		if (draggingIdx === null) return;
		const coord = svgToNormalized(e.clientX, e.clientY);
		if (!coord) return;

		// Constrain t between neighbors
		const prev = draggingIdx > 0 ? points[draggingIdx - 1].t + 0.01 : 0.01;
		const next = draggingIdx < points.length - 1 ? points[draggingIdx + 1].t - 0.01 : 0.99;
		coord.t = Math.max(prev, Math.min(next, coord.t));

		const newPoints = [...points];
		newPoints[draggingIdx] = coord;
		onpointschange?.(newPoints);
	}

	function handlePointUp(e: PointerEvent) {
		if (draggingIdx === null) return;

		// Check if dragged off-canvas → delete
		if (svgElement) {
			const rect = svgElement.getBoundingClientRect();
			if (
				e.clientX < rect.left - 20 || e.clientX > rect.right + 20 ||
				e.clientY < rect.top - 20 || e.clientY > rect.bottom + 20
			) {
				const newPoints = points.filter((_, i) => i !== draggingIdx);
				onpointschange?.(newPoints);
			}
		}

		draggingIdx = null;
	}

	// Grid lines
	let gridLines = $derived.by(() => {
		const lines: Array<{ x1: number; y1: number; x2: number; y2: number }> = [];
		// Vertical grid (time)
		for (let i = 1; i < 4; i++) {
			const x = PAD.left + (i / 4) * plotW;
			lines.push({ x1: x, y1: PAD.top, x2: x, y2: PAD.top + plotH });
		}
		// Horizontal grid (progress)
		for (let i = 1; i < 4; i++) {
			const y = PAD.top + (i / 4) * plotH;
			lines.push({ x1: PAD.left, y1: y, x2: PAD.left + plotW, y2: y });
		}
		return lines;
	});
</script>

<div class="curve-lane" class:compact>
	{#if !compact}
		<div class="lane-header">
			<label class="link-checkbox">
				<input type="checkbox" checked={linked} onchange={() => onlinkchange?.(!linked)} />
				<span>{displayName}</span>
			</label>
		</div>
	{/if}

	<!-- svelte-ignore a11y_click_events_have_key_events -->
	<!-- svelte-ignore a11y_no_static_element_interactions -->
	<svg
		viewBox="0 0 {W} {H}"
		class="lane-svg"
		bind:this={svgElement}
		onclick={handleSvgClick}
		onpointermove={handlePointMove}
		onpointerup={handlePointUp}
	>
		<!-- Plot border -->
		<rect
			x={PAD.left} y={PAD.top}
			width={plotW} height={plotH}
			fill="none" stroke="var(--color-primary)" stroke-width="0.5"
		/>

		<!-- Grid -->
		{#each gridLines as line}
			<line
				x1={line.x1} y1={line.y1} x2={line.x2} y2={line.y2}
				stroke="var(--color-primary)" stroke-width="0.3" opacity="0.4"
			/>
		{/each}

		<!-- Curve -->
		<path d={curvePath} fill="none" stroke="var(--color-accent)" stroke-width={compact ? 1.5 : 2} />

		<!-- Control points -->
		{#if !compact}
			{#each pointPositions as pos, i}
				<circle
					cx={pos.x} cy={pos.y} r="6"
					fill="var(--color-accent)" stroke="var(--color-bg)" stroke-width="1.5"
					class="control-point"
					class:dragging={draggingIdx === i}
					onpointerdown={(e) => handlePointDown(i, e)}
				/>
			{/each}
		{/if}

		<!-- Axis labels -->
		{#if !compact}
			<text x={PAD.left - 2} y={PAD.top + 4} text-anchor="end" class="axis-label">1</text>
			<text x={PAD.left - 2} y={PAD.top + plotH + 2} text-anchor="end" class="axis-label">0</text>
			<text x={PAD.left} y={PAD.top + plotH + 14} class="axis-label">0s</text>
			<text x={PAD.left + plotW} y={PAD.top + plotH + 14} text-anchor="end" class="axis-label">{durationSec.toFixed(0)}s</text>
		{/if}
	</svg>
</div>

<style>
	.curve-lane {
		display: flex;
		flex-direction: column;
	}

	.lane-header {
		display: flex;
		align-items: center;
		padding: 0.125rem 0;
	}

	.link-checkbox {
		display: flex;
		align-items: center;
		gap: 0.375rem;
		font-size: 0.75rem;
		color: var(--color-text-muted);
		text-transform: capitalize;
		cursor: pointer;
	}

	.link-checkbox input {
		accent-color: var(--color-accent);
	}

	.lane-svg {
		width: 100%;
		height: auto;
		touch-action: none;
	}

	.control-point {
		cursor: grab;
	}

	.control-point.dragging {
		cursor: grabbing;
	}

	.control-point:hover {
		r: 8;
	}

	.axis-label {
		font-size: 8px;
		fill: var(--color-text-muted);
		font-family: var(--font-mono);
	}
</style>
