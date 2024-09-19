import json
from jsonschema import validate
from dataclasses import dataclass
from typing import List, Dict

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
    dimension: Dimension

@dataclass
class RecipeNode:
    item: int
    results: List[RecipeResult]
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
        current.results.append(RecipeResult(recipe["result"], dimensions))
        current = root
    return root

def pad(indent) -> str:
    return "    " * indent

def serialiseTree(node: RecipeNode, indent = 0) -> str:
    results = "RECIPE_RESULT_LIST {\n"
    if len(node.results) > 0:
        i = 0
        for result in node.results:
            results += pad(indent + 2) + "RECIPE_RESULT_ITEM {\n"
            results += pad(indent + 3) + f".item = {result.item},\n"
            results += pad(indent + 3) + f".dimension {{{result.dimension.width}, {result.dimension.height}}}\n"
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

def main() -> None:
    schema = None
    with open("assets/recipes.schema.json", "r") as f:
        schema = json.load(f)
    recipes = None
    with open("assets/recipes.json", "r") as f:
        recipes = json.load(f)
    validate(recipes, schema)
    root = constructTree(recipes)
    tree = serialiseTree(root)
    tree = f"const RecipeNode* root = {tree}"
    # TODO: Write tree to file as: const RecipeNode* root = <TREE>

if __name__ == "__main__":
    main()
