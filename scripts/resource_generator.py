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
    opaque_bitset: str
    slipperiness: str
    hardness: str
    resistance: str
    generator: str = "block"

@dataclass
class ItemBlock:
    name: str
    generator: str = "itemblock"

def generateBlock(block: Block) -> None:
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
    filepath = "src/game/blocks/" + caseconverter.snakecase(block.name)
    with open(f"{filepath}.h", "w") as f:
        f.write(header_content)
    print(f"[INFO] Written header to {filepath}.h")
    with open(f"{filepath}.c", "w") as f:
        f.write(source_content)
    print(f"[INFO] Written source to {filepath}.c")
    print("[INFO] Update the following files accordingly:")
    print(" - src/game/blocks/block_id.c")
    print(" - src/game/blocks/blocks.h")
    print(" - src/game/blocks/blocks.c")
    print("[INFO] Generate or create an associated item block to match this on in src/game/items")

def generateItemBlock(itemblock: ItemBlock) -> None:
    pass

def main():
    dir = os.path.basename(os.getcwd())
    if (dir != "PSX-Minecraft"):
        print(f"[ERROR] Script must be run from 'PSX-Minecraft' directory, not '{dir}'")
        exit(1)
    parser = argparse.ArgumentParser(prog="resource_generator")
    sub_parsers = parser.add_subparsers(dest="generator", help="sub-command --help")

    parser_block = sub_parsers.add_parser("block", help="block --help")
    parser_block.add_argument("--name", type=str, required=True, help="name --help")
    parser_block.add_argument("--slipperiness", type=str, default="BLOCK_DEFAULT_SLIPPERINESS", help="slipperiness --help")
    parser_block.add_argument("--hardness", type=str, default="BLOCK_DEFAULT_HARDNESS", help="hardness --help")
    parser_block.add_argument("--resistance", type=str, default="BLOCK_DEFAULT_RESISTANCE", help="resistance --help")
    parser_block.add_argument("--opaque_bitset", type=str, help="opaque-bitset --help")

    parser_itemblock = sub_parsers.add_parser("itemblock", help="itemblock help")
    parser_itemblock.add_argument("--name", type=str, required=True, help="name help")

    args = vars(parser.parse_args())
    if (args["generator"] == "block"):
        generateBlock(Block(**args))
    elif (args["generator"] == "itemblock"):
        generateItemBlock(ItemBlock(**args))

if __name__ == "__main__":
    main()
