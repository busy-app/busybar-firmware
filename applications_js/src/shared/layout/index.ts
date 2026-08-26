// Screen layout: Row / Column / Stack → elements[] for DisplayDraw.
//
//     render(
//       row({ justify: 'center', align: 'baseline', gap: 1 }, [
//         { type: 'text', id: 'time', text: '12:05', font: 'bold' },
//         { type: 'text', id: 'ampm', text: 'PM', font: 'small' },
//       ]),
//     )

import type { ColumnNode, ContainerBase, Node, RowNode, StackNode, StackProps } from './types.ts'

export type {
  AnimationNode,
  ColumnNode,
  ContainerBase,
  CountdownNode,
  CrossAlign,
  ImageNode,
  LeafNode,
  MainAlign,
  Node,
  Padding,
  RectangleNode,
  RowNode,
  StackNode,
  StackProps,
  TextNode,
} from './types.ts'

export { measure, type Measured } from './measure.ts'
export { render, SCREEN, type FrameOptions } from './render.ts'

/** Props for row/column. */
type ContainerProps = ContainerBase

/** Children in a row, left to right. */
export function row(props: ContainerProps, children: Node[]): RowNode {
  return { type: 'row', ...props, children }
}

/** Children in a column, top to bottom. */
export function column(props: ContainerProps, children: Node[]): ColumnNode {
  return { type: 'column', ...props, children }
}

/** Children stacked on top of each other; the last one is on top. */
export function stack(props: StackProps, children: Node[]): StackNode {
  return { type: 'stack', ...props, children }
}
