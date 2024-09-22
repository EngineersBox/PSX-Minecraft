import json, argparse, caseconverter, os
from jsonschema import validate
from dataclasses import dataclass
from typing import List, Dict
from jinja2 import Environment, FileSystemLoader, select_autoescape
from itertools import takewhile

env = Environment(
    loader = FileSystemLoader("scripts/templates"),
    autoescape = select_autoescape()
)

# TODO: Add rest of mappings as items are added to the game
ITEM_MAPPINGS: Dict[str, int]={
    "air": 0,
    "stone": 1,
    "grass": 2,
    "dirt": 3,
    "cobblestone": 4,
    "plank": 5,
    "sapling": 6,
    "bedrock": 7,
    "water_flowing": 8,
    "water_still": 9,
    "lava_flowing": 10,
    "lava_still": 11,
    "sand": 12,
    "gravel": 13,
    "gold_ore": 14,
    "iron_ore": 15,
    "coal_ore": 16,
    "log": 17,
    "leaves": 18,
    "sponge": 19,
    "glass": 20
}

@dataclass
class Dimension:
    width: int
    height: int

@dataclass
class RecipeResult:
    item: str
    stack_size: int

@dataclass
class RecipeResults:
    dimension: Dimension
    result_count: int
    results: List[RecipeResult]

@dataclass
class RecipeNode:
    item: int
    results: List[RecipeResults]
    nodes: List["RecipeNode"]

    def add(self, item: int) -> "RecipeNode":
        if self.nodes == None:
            self.nodes = []
        found = None
        for node in self.nodes:
            if node.item == item:
                found = node
                break
        if found != None:
            return found
        new_node = RecipeNode(item, [], [])
        self.nodes.append(new_node)
        self.nodes.sort(key=lambda n: n.item)
        return new_node

def determineDimensions(pattern: List[List[str]]) -> Dimension:
    height = len(pattern)
    width = len(max(pattern, key = len))
    return Dimension(width, height)

def itemId(item: int | str) -> int:
    if type(item) == "int":
        return int(item)
    return ITEM_MAPPINGS[str(item)]

def constructTree(recipes) -> RecipeNode:
    root: RecipeNode = RecipeNode(0, [], [])
    current: RecipeNode = root
    for recipe in recipes:
        dimensions = determineDimensions(recipe["pattern"])
        for row in recipe["pattern"]:
            for item in row:
                current = current.add(itemId(item))
        if current.results == None:
            current.results = []
        results = []
        for result in recipe["results"]:
            results.append(RecipeResult(
                result["item"],
                result["stack_size"]
            ))
        current.results.append(RecipeResults(
            dimensions,
            len(recipe["results"]),
            results
        ))
        current = root
    return root

def pad(indent) -> str:
    return "    " * indent

def serialiseTree(node: RecipeNode, indent = 0) -> str:
    results = "RECIPE_RESULTS_LIST {\n"
    if len(node.results) > 0:
        i = 0
        for result in node.results:
            results += pad(indent + 2) + "RECIPE_RESULTS_ITEM {\n"
            results += pad(indent + 3) + f".dimension {{{result.dimension.width}, {result.dimension.height}}}\n"
            results += pad(indent + 3) + f".result_count = {result.result_count}\n"
            results += pad(indent + 3) + ".results = RECIPE_RESULT_LIST {\n"
            j = 0
            for _result in result.results:
                results += pad(indent + 4) + "RECIPE_RESULT_ITEM {\n"
                results += pad(indent + 5) + f".item_constructor = {_result.item}ItemConstructor,\n"
                results += pad(indent + 5) + f".stack_size = {_result.stack_size},\n"
                results += pad(indent + 4) + "}"
                if j < len(result.results) - 1:
                    results += ","
                results += "\n"
                j += 1
            results += pad(indent + 3) + "}\n"
            results += pad(indent + 2) + "}"
            if i < len(node.results) - 1:
                results += ","
            results += "\n"
            i += 1
        results += pad(indent + 1) + "}"
    else:
        results = "NULL,"
    nodes = "RECIPE_LIST {\n"
    if len(node.nodes) > 0:
        i = 0
        for _node in node.nodes:
            nodes += serialiseTree(_node, indent + 2) 
            if i < len(node.nodes) - 1:
                nodes += ","
            nodes += "\n"
            i += 1
        nodes += pad(indent + 1) + "}"
    else:
        nodes = "NULL"
    output = pad(indent) + "RECIPE_ITEM {\n"
    output += pad(indent + 1) + f".item = {node.item},\n"
    output += pad(indent + 1) + f".node_count = {len(node.nodes)},\n"
    output += pad(indent + 1) + f".result_count = {len(node.results)},\n"
    output += pad(indent + 1) + f".results = {results},\n"
    output += pad(indent + 1) + f".nodes = {nodes}\n"
    output += pad(indent) +  "}"
    return output

def generateTreeFiles(name: str, path: str, tree: str) -> None:
    render_parameters = {
        "name_snake_upper": caseconverter.macrocase(name),
        "name_snake_lower": caseconverter.snakecase(name),
        "recipe_header_include_path": os.path.relpath("src/game/recipe", path),
        "tree": tree
    }
    header_content = env.get_template("recipe_tree.h.j2").render(render_parameters)
    print("[INFO] Rendered header content")
    source_content = env.get_template("recipe_tree.c.j2").render(render_parameters)
    print("[INFO] Rendered source content")
    filepath = path + caseconverter.snakecase(name)
    with open(f"{filepath}.h", "w") as f:
        f.write(header_content)
    print(f"[INFO] Written header to {filepath}.h")
    with open(f"{filepath}.c", "w") as f:
        f.write(source_content)
    print(f"[INFO] Written source to {filepath}.c")

class IntRange:

    def __init__(self, imin=None, imax=None):
        self.imin = imin
        self.imax = imax

    def __call__(self, arg):
        try:
            value = int(arg)
        except ValueError:
            raise self.exception()
        if (self.imin is not None and value < self.imin) or (self.imax is not None and value > self.imax):
            raise self.exception()
        return value

    def exception(self):
        if self.imin is not None and self.imax is not None:
            return argparse.ArgumentTypeError(f"Must be an integer in the range [{self.imin}, {self.imax}]")
        elif self.imin is not None:
            return argparse.ArgumentTypeError(f"Must be an integer >= {self.imin}")
        elif self.imax is not None:
            return argparse.ArgumentTypeError(f"Must be an integer <= {self.imax}")
        else:
            return argparse.ArgumentTypeError("Must be an integer")

def main() -> None:
    dir = os.path.basename(os.getcwd())
    if (dir != "PSX-Minecraft"):
        print(f"[ERROR] Script must be run from 'PSX-Minecraft' directory, not '{dir}'")
        exit(1)
    parser = argparse.ArgumentParser(prog="recipe_tree")
    parser.add_argument("--recipes", type=str, required=True, help="Path to JSON specification of recipes")
    parser.add_argument("--name", type=str, required=True, help="Variable name for the tree used when declaring header")
    parser.add_argument("--output", type=str, required=True, help="Location to generate header with recipe tree")
    parser.add_argument("--min_ingredients", type=IntRange(1), required=False, help="Minimum number of ingredients to allow in recipes")
    parser.add_argument("--max_ingredients", type=IntRange(1), required=False, help="Maxmimum number of ingredients to allow in recipes")
    parser.add_argument("--pattern_width", type=IntRange(1), required=False, help="Maximum width of the recipe patterns")
    parser.add_argument("--pattern_height", type=IntRange(1), required=False, help="Maximum height of the recipe patterns")
    args = vars(parser.parse_args())
    schema = None
    with open("assets/recipes.schema.json", "r") as f:
        schema = json.load(f)
    if (args["min_ingredients"] != None):
        schema["items"]["properties"]["results"]["minItems"] = args["min_ingredients"]
    if (args["max_ingredients"] != None):
        schema["items"]["properties"]["results"]["maxItems"] = args["max_ingredients"]
    if (args["pattern_width"] != None):
        schema["item"]["properties"]["pattern"]["maxItems"] = args["pattern_width"]
    if (args["pattern_height"] != None):
        schema["item"]["properties"]["pattern"]["items"]["maxItems"] = args["pattern_height"]
    with open(args["recipes"], "r") as f:
        recipes = json.load(f)
    validate(recipes, schema)
    root = constructTree(recipes)
    print("[INFO] Constructed tree structure")
    tree = serialiseTree(root)
    print("[INFO] Serialised tree into string")
    generateTreeFiles(args["name"], args["output"], tree)

if __name__ == "__main__":
    main()
