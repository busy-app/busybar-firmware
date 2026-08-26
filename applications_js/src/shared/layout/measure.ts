// Node measurement in VISIBLE pixels. Each node carries `inkTop`, which placement subtracts from the y it emits.

import { fontBaseline, textLayoutBox, textWidth } from '../font.ts'
import { isContainer, resolvePadding, type Node } from './types.ts'

/** A node with its size computed. The tree is measured bottom-up. */
export type Measured = {
  node: Node
  /** Size of the visible pixels. */
  width: number
  height: number
  /** Empty rows above the first visible pixel. 0 for non-text. */
  inkTop: number
  /**
   * Font baseline relative to the top of the visible pixels, or `null` for non-text nodes.
   */
  baseline: number | null
  /** Measured children (empty for a leaf). */
  children: Measured[]
}

/** Size of a leaf. Text is measured; other types carry their size in props. */
function measureLeaf(node: Node): Omit<Measured, 'node' | 'children'> {
  if (node.type === 'text') {
    // Runs to the baseline, excluding descenders: see textLayoutBox.
    const box = textLayoutBox(node.text, node.font)
    return {
      width: textWidth(node.text, node.font),
      height: box.height,
      inkTop: box.top,
      // From the top of the visible pixels, not of the line.
      baseline: fontBaseline(node.font) - box.top,
    }
  }
  // Optional to TS, guaranteed by the concrete node types.
  const sized = node as { width?: number; height?: number }
  return { width: sized.width ?? 0, height: sized.height ?? 0, inkTop: 0, baseline: null }
}

/**
 * Measures the tree bottom-up. Containers without an explicit size fit their content. A row's height is (max above the baseline) + (max below it).
 */
export function measure(node: Node): Measured {
  if (!isContainer(node)) {
    return { node, ...measureLeaf(node), children: [] }
  }

  const children = node.children.map(measure)
  const pad = resolvePadding(node.padding)
  const gap = node.type === 'stack' ? 0 : (node.gap ?? 0)
  const gaps = children.length > 1 ? gap * (children.length - 1) : 0

  let contentW: number
  let contentH: number
  // Inherited from the first text child.
  let baseline: number | null = null

  if (node.type === 'row') {
    contentW = children.reduce((sum, c) => sum + c.width, 0) + gaps

    // Children with a baseline line up on it; the rest take their height.
    const above = Math.max(0, ...children.map((c) => c.baseline ?? c.height))
    const below = Math.max(0, ...children.map((c) => c.height - (c.baseline ?? c.height)))
    contentH = above + below
    baseline = children.some((c) => c.baseline !== null) ? above : null
  } else if (node.type === 'column') {
    contentW = children.reduce((max, c) => Math.max(max, c.width), 0)
    contentH = children.reduce((sum, c) => sum + c.height, 0) + gaps
    // A column's baseline is its first row's.
    baseline = children.length > 0 ? children[0]!.baseline : null
  } else {
    contentW = children.reduce((max, c) => Math.max(max, c.width), 0)
    contentH = children.reduce((max, c) => Math.max(max, c.height), 0)
  }

  return {
    node,
    width: node.width ?? contentW + pad.left + pad.right,
    height: node.height ?? contentH + pad.top + pad.bottom,
    // Not propagated outward: the parent sees a solid block.
    inkTop: 0,
    baseline: baseline === null ? null : baseline + pad.top,
    children,
  }
}
