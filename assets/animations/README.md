# Animations

## File format

Animations are regular `.zip` archives with the following requirements:

- Archive name SHALL follow the template: `animation_name_WxH.zip`, where `W` and `H` are frame width and height in pixels.
- Archive file MUST have a SINGLE top directory that has the same basename as the file itself (minus the `.zip` suffix).
- Top directory MUST contain one or more `.png` images which SHALL be of the same dimensions.
- Image names SHALL follow the template: `frame_$number.png`, where the `$number` part is a non-negative number starting from zero, without leading zeroes.
- Image numbers SHALL follow a natural progression in the file names, e.g. `frame_0, frame_1, ... frame_9, frame_10, ... frame_199, frame_200`, etc.
- Top directory MUST contain a file named `meta.txt`. The format of this file is TBD.

### Example archive structure

```
 +-animation_name_WxH.zip
    |
    +-animation_name_WxH
       |
       +-meta.txt
       +-frame_0.png
       +-frame_1.png
       +- ....
       +-frame_n.png
```

## Directory structure

The hierarchy created under the `assets/animations` directory will be replicated on the device under `/ext/apps_assets`, with `*.zip` source archives replaced by compiled `*.anim` files.
