import re, os 
from pathlib import Path

ORDERING_PREFIX_PATTERN: re.Pattern = re.compile(r"^(\d+_)?")

class Textures:
    mappings: dict[str, tuple[str, str]]

    def __init__(self):
        self.mappings = {}
        self.resolve_mappings()

    def resolve_mappings(self):
        for _ , dirs, _ in os.walk(u"../../assets"):
            for dir in dirs:
                dir_path = Path(f"../../assets/{dir}").absolute()
                for f in os.listdir(dir_path):
                    tim_path = Path(f"{dir_path}/{f}")
                    if tim_path.suffix != ".tim":
                        continue
                    file_name = re.sub(
                        ORDERING_PREFIX_PATTERN,
                        '',
                        tim_path.name.removesuffix(".tim").upper()
                    )
                    try:
                        png_name = tim_path.name.removesuffix(".tim") + ".png"
                        png_path = Path(f"{dir_path}/{png_name}")
                        if not png_path.exists():
                            continue
                        self.mappings[f"ASSET_TEXTURE__{dir.upper()}__{file_name}"] = (f"{dir.upper()}__{file_name}", str(png_path))
                    except ValueError:
                        continue

    def tupled_keys(self) -> list[str | tuple[str, str]]:
        keys: list[str | tuple[str, str]] = []
        for (key, value) in self.mappings.items():
            keys.append((value[0], key))
        return keys
