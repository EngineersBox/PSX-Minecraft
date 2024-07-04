from jinja2 import Environment, FileSystemLoader, select_autoescape
from dataclasses import dataclass
import argparse, caseconverter, os

env = Environment(
    loader = FileSystemLoader("scripts/templates"),
    autoescape = select_autoescape()
)

@dataclass
class Block:
    name: str
    orientation: str
    type: str
    face_attributes: str
    tinted_face_attributes: str
    opaque_bitset: str
    slipperiness: str
    hardness: str
    resistance: str
    stateful: bool
    tool_type: str
    tool_material: str
    can_harvest_bitset: str
    has_use_action: bool
    generator: str = "block"

@dataclass
class ItemBlock:
    name: str
    max_stack_size: str
    face_attributes: str
    tinted_face_attributes: str
    generator: str = "itemblock"

def generateBlock(block: Block) -> None:
    if ((block.face_attributes != None) == (block.tinted_face_attributes != None)):
        print("[ERROR] Cannot have both tinted and non-tinted face attributes")
        exit(1)
    render_parameters = {
        "name_snake_upper": caseconverter.macrocase(block.name),
        "name_snake_lower": caseconverter.snakecase(block.name),
        "name_lower": caseconverter.camelcase(block.name),
        "name_capital": caseconverter.pascalcase(block.name),
        **vars(block)
    }
    header_content = env.get_template("block.h.j2").render(render_parameters)
    print("[INFO] Rendered header content")
    source_content = env.get_template("block.c.j2").render(render_parameters)
    print("[INFO] Rendered source content")
    filepath = "src/game/blocks/block_" + caseconverter.snakecase(block.name)
    with open(f"{filepath}.h", "w") as f:
        f.write(header_content)
    print(f"[INFO] Written header to {filepath}.h")
    with open(f"{filepath}.c", "w") as f:
        f.write(source_content)
    print(f"[INFO] Written source to {filepath}.c")
    print("[INFO] Update the following files accordingly:")
    print(f" - \"src/game/blocks/block_id.c\" to declare BLOCKID_{render_parameters["name_snake_upper"]}")
    print(f" - \"src/game/blocks/blocks.h\" to add the include via '#include \"block_{render_parameters["name_snake_lower"]}.h\"'")
    if (not block.stateful):
        print(" - \"src/game/blocks/blocks.c\" to declare a static instance of the block")
    print("[INFO] If required, generate or create an associated item block to match this in \"src/game/items\"")

def generateItemBlock(itemblock: ItemBlock) -> None:
    if ((itemblock.face_attributes != None) == (itemblock.tinted_face_attributes != None)):
        print("[ERROR] Cannot have both tinted and non-tinted face attributes")
        exit(1)
    render_parameters = {
        "name_snake_upper": caseconverter.macrocase(itemblock.name),
        "name_snake_lower": caseconverter.snakecase(itemblock.name),
        "name_lower": caseconverter.camelcase(itemblock.name),
        "name_capital": caseconverter.pascalcase(itemblock.name),
        **vars(itemblock)
    }
    header_content = env.get_template("item_block.h.j2").render(render_parameters)
    print("[INFO] Rendered header content")
    source_content = env.get_template("item_block.c.j2").render(render_parameters)
    print("[INFO] Rendered source content")
    filepath = "src/game/items/blocks/item_block_" + caseconverter.snakecase(itemblock.name)
    with open(f"{filepath}.h", "w") as f:
        f.write(header_content)
    print(f"[INFO] Written header to {filepath}.h")
    with open(f"{filepath}.c", "w") as f:
        f.write(source_content)
    print(f"[INFO] Written source to {filepath}.c")
    print("[INFO] Update the following files accordingly:")
    print(f" - \"src/game/items/item_id.h\" to declare ITEMID_{render_parameters["name_snake_upper"]}")
    print(f" - \"src/game/items/blocks/item_block_{render_parameters["name_snake_lower"]}.h\" implement functions marked with UNIMPLEMENTED()")
    print("[INFO] If required, generate or create an associated block to match this in \"src/game/blocks\"")

def main():
    dir = os.path.basename(os.getcwd())
    if (dir != "PSX-Minecraft"):
        print(f"[ERROR] Script must be run from 'PSX-Minecraft' directory, not '{dir}'")
        exit(1)
    parser = argparse.ArgumentParser(prog="resource_generator")
    sub_parsers = parser.add_subparsers(dest="generator", help="sub-command --help")

    # ==== BLOCK ====
    parser_block = sub_parsers.add_parser("block", help="block --help")
    parser_block.add_argument("--name", type=str, required=True, help="Name of the block")
    parser_block.add_argument(
        "--orientation",
        type=str,
        default="FACE_DIR_RIGHT",
        choices=[
            "FACE_DIR_DOWN",
            "FACE_DIR_UP",
            "FACE_DIR_LEFT",
            "FACE_DIR_RIGHT",
            "FACE_DIR_BACK",
            "FACE_DIR_FRONT"
        ],
        help="Orientation the block should be in for texture indexing"
    )
    parser_block.add_argument(
        "--type",
        type=str,
        default="BLOCKTYPE_SOLID",
        choices=[
            "BLOCKTYPE_EMPTY",
            "BLOCKTYPE_SOLID",
            "BLOCKTYPE_STAIR",
            "BLOCKTYPE_SLAB",
            "BLOCKTYPE_CROSS",
            "BLOCKTYPE_HASH",
            "BLOCKTYPE_PLATE",
            "BLOCKTYPE_ROD",
            "BLOCKTYPE_LIQUID"
        ],
        help="Block type to determine mesh and characteristics"
    )
    parser_block.add_argument("--face_attributes", type=int, default=None, help="Texture page index for 16x16 face texture on all sides [NOTE: Exclusive with --tinited_face_attributes]")
    parser_block.add_argument("--tinted_face_attributes", type=str, default=None, help="Per-face (6) texture page indices and optional tint formatted as '<down>,<up>,<left>,<right>,<front>,<back>'. Where each entry is comma separated and of the form '<index>, NO_TINT' or '<index>, faceTint(r,g,b,cd)' [NOTE: Exclusive with --face_attributes]")
    parser_block.add_argument("--slipperiness", type=str, default="BLOCK_DEFAULT_SLIPPERINESS", help="How much physics objects slide on the block (player, items, mobs, etc)")
    parser_block.add_argument("--hardness", type=str, default="BLOCK_DEFAULT_HARDNESS", help="How difficult it is to use the relevant tool on")
    parser_block.add_argument("--resistance", type=str, default="BLOCK_DEFAULT_RESISTANCE", help="How resistant it is to explosions")
    parser_block.add_argument("--opaque_bitset", type=str, help="Face specific opacity where 1=opaque and 0=transparent of the form '<down>,<up>,<left>,<right>,<front>,<back>'")
    parser_block.add_argument("--stateful", type=bool, action=argparse.BooleanOptionalAction, default=False, help="Whether the block should maintain state or be static (single instance)")
    parser_block.add_argument(
        "--tool_type",
        type=str,
        default="TOOLTYPE_NONE",
        choices=[
            "TOOLTYPE_PICKAXE",
            "TOOLTYPE_AXE",
            "TOOLTYPE_SWORD",
            "TOOLTYPE_SHOVEL",
            "TOOLTYPE_HOE",
            "TOOLTYPE_NONE"
        ],
        help="Type of tool that is used to mine the block"
    )
    parser_block.add_argument(
        "--tool_material",
        type=str,
        default="ITEMMATERIAL_NONE",
        choices=[
            "ITEMMATERIAL_NONE",
            "ITEMMATERIAL_WOOD",
            "ITEMMATERIAL_STONE",
            "ITEMMATERIAL_IRON",
            "ITEMMATERIAL_GOLD",
            "ITEMMATERIAL_DIAMOND"
        ],
        help="Minimum material level of the tool used to mine this block"
    )
    parser_block.add_argument("--can_harvest_bitset", type=str, required=True, help="Tool specific flags to determine if the tool can mine the block where 1=true and 0=false of the form '<none>,<pickaxe>,<axe>,<sword>,<shovel>,<hoe>'")
    parser_block.add_argument("--has_use_action", type=bool, action=argparse.BooleanOptionalAction, default=False, help="Whether the block should respond to use actions")

    # ==== ITEM BLOCK ====
    parser_itemblock = sub_parsers.add_parser("itemblock", help="itemblock help")
    parser_itemblock.add_argument("--name", type=str, required=True, help="Name of the item block")
    parser_itemblock.add_argument("--max_stack_size", type=str, default=64, help="Maximum item count to have in a stack")
    parser_itemblock.add_argument("--face_attributes", type=int, default=None, help="Texture page index for 16x16 face texture on all sides [NOTE: Exclusive with --tinited_face_attributes]")
    parser_itemblock.add_argument("--tinted_face_attributes", type=str, default=None, help="Per-face (6) texture page indices and optional tint formatted as '<down>,<up>,<left>,<right>,<front>,<back>'. Where each entry is comma separated and of the form '<index>, NO_TINT' or '<index>, faceTint(r,g,b,cd)' [NOTE: Exclusive with --face_attributes]")

    args = vars(parser.parse_args())
    if (args["generator"] == "block"):
        generateBlock(Block(**args))
    elif (args["generator"] == "itemblock"):
        generateItemBlock(ItemBlock(**args))

if __name__ == "__main__":
    main()
