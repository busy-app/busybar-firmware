# Animations

## File format

Animations are regular `.zip` archives with the following requirements:

- Archive name SHALL follow the template: `animation_name_WxH.zip`, where `W` and `H` are frame width and height in pixels.
- Archive file MUST have a SINGLE top directory that has the same basename as the file itself (minus the `.zip` suffix).
- Top directory MUST contain one or more `.png` images which SHALL be of the same dimensions.
- Image names SHALL follow the template: `frame_$number.png`, where the `$number` part is a non-negative number starting from zero, without leading zeroes.
- Image numbers SHALL follow a natural progression in the file names, e.g. `frame_0, frame_1, ... frame_9, frame_10, ... frame_199, frame_200`, etc.
- Top directory MUST contain a file named `meta.json`. See [Metadata](#metadata)

### Metadata

The metadata file MUST have the name `meta.json` as described in the previous section.

Example:
```json
{
   "fps": 30,
   "color": "rgb888",
   "sections": [
      {
         "name": "whole",
         "start": 0,
         "end": 14
      },
      {
         "name": "first_section",
         "start": 0,
         "end": 10
      },
      {
         "name": "second_section",
         "start": 5,
         "end": 14
      }
   ]
}
```

_Note: [JSON5](https://json5.org/) is planned to be supported in the future. In the meantime,
trailing commas will result in a build error._

- `fps`: Frames per Second for the entire animation.
- `color`: Color packing mode. Either mode can be played on either display, but `"rgb888"` is
  suggested for the front display and `"gray4"` for the back one.
- `sections`: List of sections. Frame indices are inclusive on both ends. Sections MAY overlap.
  Order of elements in this array is preserved when generating the file. Section with index 0 MUST
  be present, MUST have the name `"whole"` and MUST cover the entire range of frames.

### Menu metadata
In order to use an `.anim` file with the `AnimMenu` widget, specific sections must be described in
`meta.json`:
  - Selected item without any input: `item-X` (X is the index of the menu entry)
  - Transition between items: `transition-X-to-Y` (X and Y are indices). Transitions are always
    between two adjacent items, i.e. you should define `transition-6-to-7`, but
    `transition-7-to-11` will never be used.

The indices of these special sections do not matter, only the name.

### Example archive structure

```
 +-animation_name_WxH.zip
    |
    +-animation_name_WxH
       |
       +-meta.json
       +-frame_0.png
       +-frame_1.png
       +- ....
       +-frame_n.png
```

## Directory structure

The hierarchy created under the `assets/animations` directory will be replicated on the device under `/ext/apps_assets`, with `*.zip` source archives replaced by compiled `*.anim` files.
