# Texture Wrapping

Using the `DR_TWIN` primitive to set the texture windowing, we can define how a
textured primitive wraps its UVs when outside a given range. `DR_TWIN` is as follows,
according to the spec <https://psx-spx.consoledev.net/graphicsprocessingunitgpu/#gp0e2h-texture-window-setting>:

```
0-4    Texture window Mask X   (in 8 pixel steps)
5-9    Texture window Mask Y   (in 8 pixel steps)
10-14  Texture window Offset X (in 8 pixel steps)
15-19  Texture window Offset Y (in 8 pixel steps)
20-23  Not used (zero)
24-31  Command  (E2h)
```

We can visualise the wrapping with the following snippet

```rust
struct Coord {
    x_min: u16,
    x_max: u16,
    y_min: u16,
    y_max: u16
}

struct Window {
    mask_x: u16,
    mask_y: u16,
    offset_x: u16,
    offset_y: u16
}

fn main() {
    let coords: Coord = Coord {
        x_min: 320,
        x_max: 368,
        y_min: 16,
        y_max: 48
    };
    let window: Window = Window {
        mask_x: 0x2,
        mask_y: 0x2,
        offset_x: 0x0,
        offset_y: 0x2
    };
    print!("  ");
    for coord in (coords.x_min)..(coords.x_max) {
        print!(" {:02}", (coord & ((window.mask_x * 0x8) - 1)) | ((window.offset_x & window.mask_x) * 0x8));
    }
    print!("\n");
    for coord in (coords.y_min)..(coords.y_max) {
        println!("{:02}", (coord & ((window.mask_y * 0x8) - 1)) | ((window.offset_y & window.mask_y) * 0x8));
    }
}
```

Which prints the following show wrapping every 16 pixels on the X-axis and every 16 pixels on the Y-axis
for a primitive that is 48x32.

```
   00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15
16
17
18
19
20
21
22
23
24
25
26
27
28
29
30
31
16
17
18
19
20
21
22
23
24
25
26
27
28
29
30
31
```