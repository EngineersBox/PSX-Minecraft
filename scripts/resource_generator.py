from jinja2 import Environment, FileSystemLoader, select_autoescape
import argparse

env = Environment(
    loader = FileSystemLoader("templates"),
    autoescape = select_autoescape()
)

def generateBlock(name: str, attributes: dict[str, str]) -> None:
    block_name_snake_upper = "BLOCK_NAME"
    block_name_snake_lower = "block_name"
    block_name_lower = "blockName"
    block_name_capital = "BlockName"
    attributes = {
        "slipperiness": "BLOCK_DEFAULT_SLIPPERINESS",
        "hardness": "BLOCK_DEFAULT_HARDNESS",
        "resistance": "BLOCK_DEFAULT_RESISTANCE"
    }
    header_content = env.get_template("block.h.j2").render(...)
    source_content = env.get_template("block.c.j2").render(...)


def generateItemBlock() -> None:
    pass

def main():
    parser = argparse.ArgumentParser(prog="resource_generator")
    sub_parsers = parser.add_subparsers(help="sub-command help")

    parser_block = subparsers.add_parser("block", help="block help")
    parser_block.add_argument("--name", type=str, help="name help")

    parser_itemblock = sub_parsers.add_parser("itemblock", help="itemblock help")
    parser_itemblock.add_argument("--name", type=str, help="name help")

    args = parser.parse_args()
    pass

if __name__ == "__main__":
    main()