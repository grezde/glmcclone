{
    model: "cuboids",
    texture: "entities/cow",
    movement: "quadruped",
    aabb: { start: (-0.8, 0, -0.6), dims: (1.6, 1.0, 1.2) },
    cuboids: {
        body: {
            // uv represents the top-right corner of the face where the unwrapping begins
            uv: (28, 14),
            // dims represent: width of unwrapping face, height of unwrapping face, the other dimension
            // by default, the unwrapping face is towards the east, in the up direction
            dims: (12, 18, 10)
        },
        leg: { uv: (4, 20), dims: (4, 12, 4) },
        head: { uv: (6, 6), dims: (8, 8, 6) },
        horn: { uv: (23, 1), dims: (1, 3, 1) },
        udder: { uv: (53, 1), dims: (4, 6, 1) }
    },
    objects: {
        body: {
            cuboids: [ 
                { id: "body", pos: (0, 17, 0), facing: up, downwards: east },
                { id: "udder", pos: (-6, 11.5, 0), facing: down, downwards: east }
            ]
        },
        head: {
            pivot: (11, 20, 0),
            cuboids: [ 
                { id: "head", pos: (12, 20, 0) }, 
                { id: "horn", pos: (11.5, 23.5, -5.5) }, 
                { id: "horn", pos: (11.5, 23.5, 4.5), flip: true }
            ]
        },
        leg0: {
            pivot: (-6, 12, -4),
            cuboids: [ { id: "leg", pos: (-6, 6, -4), flip: true } ]
        },
        leg1: {
            pivot: (-6, 12, 4),
            cuboids: [ { id: "leg", pos: (-6, 6, 4) } ]
        },
        leg2: {
            pivot: (6, 12, -4),
            cuboids: [ { id: "leg", pos: (6, 6, -4), flip: true } ]
        },
        leg3: {
            pivot: (6, 12, 4),
            cuboids: [ { id: "leg", pos: (6, 6, 4) } ]
        }
    }
}