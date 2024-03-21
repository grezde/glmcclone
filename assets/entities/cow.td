{
    model: "cuboid_custom",
    texture: "entities/cow",
    movement: "quadruped",
    aabb: { start: (-1.0, -1.0, -1.0), dims: (2.0, 2.0, 2.0) },
    cuboids: {
        body: { up: (28, 14), down: (50, 14), south: (18, 14), north: (40, 14), east: (28,  4), west: (40,  4), dims: (18, 12, 10) },
        leg:  { up: ( 4, 16), down: ( 8, 16), south: ( 0, 20), north: ( 8, 20), east: ( 4, 20), west: (12, 20), dims: ( 4, 12,  4) },
        head: { up: ( 6,  0), down: (14,  0), south: ( 0,  6), north: (14,  6), east: ( 6,  6), west: (20,  6), dims: ( 6,  8,  8) },
        horn: { up: (23,  0), down: (24,  0), south: (22,  1), north: (24,  1), east: ( 6,  1), west: (25,  1), dims: ( 1,  1,  1) }
    },
    objects: {
        body: {
            cuboids: [ { id: "body", pos: (-9, 12, -5) }]
        },
        head: {
            pivot: (11, 20, 0),
            cuboids: [ 
                { id: "head", pos: (9, 16, -4) }, 
                { id: "horn", pos: (11, 22, -5) }, 
                { id: "horn", pos: (11, 22, 4), flip_x: true } 
            ]
        },
        leg0: {
            pivot: (-6, 12, -4),
            cuboids: [ { id: "leg", pos: (-8, 0, -6) } ]
        },
        leg1: {
            pivot: (-6, 12, 4),
            cuboids: [ { id: "leg", pos: (-8, 0, 2) } ]
        },
        leg2: {
            pivot: (6, 12, -4),
            cuboids: [ { id: "leg", pos: (4, 0, -6) } ]
        },
        leg3: {
            pivot: (6, 12, 4),
            cuboids: [ { id: "leg", pos: (4, 0, 2) } ]
        }
    }
}