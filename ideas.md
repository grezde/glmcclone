
# Ideas

## TODOS

- Check faces on chunk edges
- Add a a direction parameter to the faces of the cube model
- 
- Implement multi-threading for saving, rendering and updating
- Consider adding multiplayer once threading is done

## Block data enum

Block data enums are u8 enumerations that are attached to blocks.
For example, block enums include: 
- `colors`: colored blocks (like wool, concrete, beds)
- `light_colors`: colored light-emitting blocks (like lamps) 
- `wood_variants`: what wood variants to use (like wood)
- `orientations`: which orientation does the block face (like pistons)
- `fullness`: used in tiny cubes to select the shape (like the chese block in april fools 2023)
- `pallete`: used in tiny cubes to select the texture
A block can have multiple enums: beds have `wood_variant` and `colors` enums, a storage drawer (like in the mod) may have 2 `wood_variant` enums. A tiny block may have a `fullness` and a `pallete` enum.