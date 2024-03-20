#pragma once

namespace Block {
  enum block_t  {
	none, grass, dirt, stone, cobblestone, gravel, sand,
	oak, plank_oak, leaf_oak, bedrock, brick, glass, tnt, pumpkin, jacklantern,
	tourch, ore_iron, ore_gold, ore_diamond, ore_redstone, ore_coal,
	mushroom_red, mushroom_brown, web, lava, water,
	weed0, weed1, weed2, weed3, rose, yellowflower, sapling_oak,
	wheat0, wheat1, wheat2, wheat3, wheat4, wheat5, wheat6, wheat7, wheat8,
	farmland, farmland_wet, snow, ice, wool,
	crafting_table, furnace, furnace_lit, rail, rail_turn, block_type_total
  };
}
namespace BlockShape {
  // shape0 = block, shape1 = plant, shape2 = wheat
  enum block_shape_t  {
	cube, cross, shape2, shape3, block_shape_total
  };
}