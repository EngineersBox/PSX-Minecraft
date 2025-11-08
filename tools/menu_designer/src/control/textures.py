import re, os
from pathlib import Path

ORDERING_PREFIX_PATTERN: re.Pattern = re.compile(r"^(\d+_)?")

class Textures:
    mappings: dict[str, str]

    def __init__(self):
        self.mappings = {}
        self.resolve_mappings()

    def resolve_mappings(self):
        for _ , dirs, _ in os.walk(u"assets"):
            for dir in dirs:
                dir_path = Path(f"./assets/{dir}").absolute()
                for f in os.listdir(dir_path):
                    tim_path = dir_path.relative_to(f"./{f}")
                    if tim_path.suffix != ".tim":
                        continue
                    file_name = re.sub(
                        ORDERING_PREFIX_PATTERN,
                        '',
                        tim_path.name.upper()
                    )
                    try:
                        png_path = dir_path.relative_to(f"{tim_path.name}.png")
                        self.mappings[f"ASSERT_TEXTURE__{dir.upper()}__{file_name}"] = str(png_path)
                    except ValueError:
                        continue

