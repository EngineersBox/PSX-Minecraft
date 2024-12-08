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

#[inline]
fn is_log2(v: u8) -> bool {
    return (v & (v - 1)) == 0;
}

fn extract_mask(coord: u16) -> u8 {
    if coord >= 32 && is_log2((coord >> 4) as u8) {
        return 0b00010;
    }
    return 0b11110;
}

fn extract_offset(coord: u16) -> u8 {
    if coord >= 32 && is_log2((coord >> 4) as u8) {
        return 0b00000;
    }
    return (coord >> 3) as u8;
}

fn main() {
    // let u: u16 = 11 * 16;
    // let v: u16 = 2 * 16;
    // let w: u16 = 16;
    // let h: u16 = 16;
    let coords: Coord = Coord {
        x_min: 0,//176,
        x_max: 32,//208,
        y_min: 16,//48,
        y_max: 48,//64
    };
    let window: Window = Window {
        mask_x: extract_mask(coords.x_min) as u16,
        mask_y: extract_mask(coords.y_min) as u16,
        offset_x: extract_offset(coords.x_min) as u16,
        offset_y: extract_offset(coords.y_min) as u16
    };
    // ((  0 + x) & (0xff - (0b11110 << 3))) | (( 0b00000 & 0b11110) << 3)
    // (( 16 + x) & (0xff - (0b11110 << 3))) | (( 0b00010 & 0b11110) << 3)
    // (( 32 + x) & (0xff - (0b00010 << 3))) | (( 0b00000 & 0b00010) << 3)
    // (( 48 + x) & (0xff - (0b11110 << 3))) | (( 0b00110 & 0b11110) << 3)
    // (( 64 + x) & (0xff - (0b00010 << 3))) | (( 0b00000 & 0b00010) << 3)
    // (( 80 + x) & (0xff - (0b11110 << 3))) | (( 0b01010 & 0b11110) << 3)
    // (( 96 + x) & (0xff - (0b11110 << 3))) | (( 0b01100 & 0b11110) << 3)
    // ((112 + x) & (0xff - (0b11110 << 3))) | (( 0b01110 & 0b11110) << 3)
    // ((128 + x) & (0xff - (0b00010 << 3))) | (( 0b00000 & 0b00010) << 3)
    // ((144 + x) & (0xff - (0b11110 << 3))) | (( 0b10010 & 0b11110) << 3)
    // ((160 + x) & (0xff - (0b11110 << 3))) | (( 0b10100 & 0b11110) << 3)
    // ((176 + x) & (0xff - (0b11110 << 3))) | (( 0b10110 & 0b11110) << 3)
    // ((192 + x) & (0xff - (0b11110 << 3))) | (( 0b11000 & 0b11110) << 3)
    // ((208 + x) & (0xff - (0b11110 << 3))) | (( 0b11010 & 0b11110) << 3)
    // ((224 + x) & (0xff - (0b11110 << 3))) | (( 0b11100 & 0b11110) << 3)
    // ((240 + x) & (0xff - (0b11110 << 3))) | (( 0b11110 & 0b11110) << 3)
    // 256 is out of range as log_2(256) > 15
    println!("[X: {}] Mask: {:#07b} Offset: {:#07b}", coords.x_min, window.mask_x, window.offset_x);
    println!("[Y: {}] Mask: {:#07b} Offset: {:#07b}", coords.y_min, window.mask_y, window.offset_y);
    print!("  ");
    for coord in (coords.x_min)..(coords.x_max) {
        print!(" {:02}", (coord & (!(window.mask_x << 3))) | ((window.offset_x & window.mask_x) << 3));
    }
    print!("\n");
    for coord in (coords.y_min)..(coords.y_max) {
        println!("{:02}", (coord & (!(window.mask_y << 3))) | ((window.offset_y & window.mask_y) << 3));
    }
}

