// Layout tree types. The tree exists only in TS: the engine flattens it into DisplayDraw's elements[]. Leaf fields come from busy-lib and pass through, minus x/y/align, plus sizes where needed.

import type { DisplayDrawParams, DisplayElement } from '@busy-app/busy-lib'

/** Not in @busy-app/busy-lib 0.17.0 yet. Drop once the library ships it. */
type XpmBitmapElement = Omit<DisplayElement, 'type'> & {
  /**
   * @description XPM2 source as plain text: must begin with a "! XPM2" signature line, followed by a "width height ncolors cpp" header line, then ncolors color lines, then height pixel rows.
   */
  data: string
  /** @description Opacity of the image in percentage (0-100) */
  opacity?: number
} & {
  /** @enum {string} */
  type: 'xpmbitmap'
}

// The discriminated union lives in the DisplayDraw params, not in DisplayElement.
type Element = NonNullable<DisplayDrawParams['elements']>[number] | XpmBitmapElement

type ByType<T extends Element['type']> = Extract<Element, { type: T }>

/** Makes the listed fields optional. */
type Optional<T, K extends PropertyKey> = Omit<T, K> &
  Partial<Pick<T, Extract<K, keyof T>>>

/** Computed by the engine; not settable by hand. */
type Computed = 'x' | 'y' | 'align'

/** Required in busy-lib's types, but defaulted by the schema. */
type Defaulted = 'display' | 'color' | 'opacity' | 'fill' | 'fill_colors' | 'border_width' | 'border_color'

/** A DisplayDraw leaf without computed fields and with defaults optional. */
type Leaf<T> = T extends unknown ? Optional<Omit<T, Computed>, Defaulted> : never

/** A size the engine can't measure itself. Distributive to preserve unions. */
type WithSize<T> = T extends unknown ? T & { width: number; height: number } : never

/** Text. The engine measures it using the element's own `font`. */
export type TextNode = Leaf<ByType<'text'>>

/** Image. `width`/`height` are explicit: the device doesn't report sizes. */
export type ImageNode = WithSize<Leaf<ByType<'image'>>>

/** Animation. Explicit sizes, as for images. */
export type AnimationNode = WithSize<Leaf<ByType<'animation'>>>

/** Rectangle: width/height are already part of the element. */
export type RectangleNode = Leaf<ByType<'rectangle'>>

/** Image from XPM2 source in `data`. Explicit sizes, as for images. */
export type XpmBitmapNode = WithSize<Leaf<ByType<'xpmbitmap'>>>

/** Countdown. Explicit size: the text lives inside the firmware. */
export type CountdownNode = WithSize<Leaf<ByType<'countdown'>>>

/** A leaf: what becomes a DisplayDraw element. */
export type LeafNode = Offset &
  (TextNode | ImageNode | AnimationNode | RectangleNode | CountdownNode | XpmBitmapNode)

/** Manual offset from where the container placed the node. */
export type Offset = { dx?: number; dy?: number }

/** Alignment along the container's cross axis. */
export type CrossAlign =
  | 'start'
  | 'center'
  | 'end'
  /** On the text baseline. Behaves as `end` for non-text nodes. */
  | 'baseline'

/** Distribution along the container's main axis. */
export type MainAlign =
  | 'start'
  | 'center'
  | 'end'
  /** Equal gaps between children, the outermost flush with the edges. */
  | 'between'

/** Padding inside a container. A number applies to all sides. */
export type Padding =
  | number
  | { top?: number; right?: number; bottom?: number; left?: number }

export type ContainerBase = Offset & {
  /** Gap between children along the main axis. */
  gap?: number
  padding?: Padding
  justify?: MainAlign
  align?: CrossAlign
  /** Fixed size. Defaults to fitting the content. */
  width?: number
  height?: number
}

/** Children in a row, left to right. */
export type RowNode = ContainerBase & {
  type: 'row'
  children: Node[]
}

/** Children in a column, top to bottom. */
export type ColumnNode = ContainerBase & {
  type: 'column'
  children: Node[]
}

/** Stack props: a container without gap. */
export type StackProps = Omit<ContainerBase, 'gap'>

/**
 * Children overlapping in one coordinate space; array order is draw order, so the last child is on top. `justify` is horizontal, `align` vertical.
 */
export type StackNode = StackProps & {
  type: 'stack'
  children: Node[]
}

export type ContainerNode = RowNode | ColumnNode | StackNode

export type Node = LeafNode | ContainerNode

export function isContainer(node: Node): node is ContainerNode {
  return node.type === 'row' || node.type === 'column' || node.type === 'stack'
}

/** Normalizes padding to four sides. */
export function resolvePadding(padding: Padding | undefined): {
  top: number
  right: number
  bottom: number
  left: number
} {
  if (padding === undefined) return { top: 0, right: 0, bottom: 0, left: 0 }
  if (typeof padding === 'number') {
    return { top: padding, right: padding, bottom: padding, left: padding }
  }
  return {
    top: padding.top ?? 0,
    right: padding.right ?? 0,
    bottom: padding.bottom ?? 0,
    left: padding.left ?? 0,
  }
}
