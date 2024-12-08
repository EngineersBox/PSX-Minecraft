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
    let u: u16 = 11 * 16;
    let v: u16 = 2 * 16;
    let w: u16 = 16;
    let h: u16 = 16;
    let coords: Coord = Coord {
        x_min: 176,
        x_max: 208,
        y_min: 32,
        y_max: 64
    };
    let window: Window = Window {
        mask_x: 0b1110,
        mask_y: 0b0010,
        offset_x: 0b0110,
        offset_y: 0b0000
    };
    print!("  ");
    for coord in (coords.x_min)..(coords.x_max) {
        print!(" {:02}", (coord & (!(window.mask_x * 0x8))) | ((window.offset_x & window.mask_x) * 0x8));
    }
    print!("\n");
    for coord in (coords.y_min)..(coords.y_max) {
        println!("{:02}", (coord & (!(window.mask_y * 0x8))) | ((window.offset_y & window.mask_y) * 0x8));
    }
}

