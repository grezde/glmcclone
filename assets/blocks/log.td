{
    model: "cube",
    textures: {
        x: "blocks/log_side",
        y: "blocks/log_side",
        z: "blocks/log_side",
        orient_x: up,
        orient_y: east,
        orient_z: up
    },
    variants: {
        "x": { textures: { x: "blocks/log_top", orient_y: east, orient_z: east }},
        "y": { textures: { y: "blocks/log_top" }},
        "z": { textures: { z: "blocks/log_top", orient_y: north, orient_x: north }},
    }
}