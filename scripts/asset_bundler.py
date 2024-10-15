# 1. Iterate subdirectories of ./assets
# 2. Each subdirectory becomes a distct bundle
# 3. Each file in the subdirectory becomes an asset <file>
# 4. If there are nested subdirectories (only one level) then
#    pack them into nested archive <file>'s with assets within them
import os, logging, logging.config, coloredlogs
from typing import Tuple
import xml.etree.ElementTree as ET
from pathlib import Path

class ColoredFormatter(coloredlogs.ColoredFormatter):
    def __init__(self, fmt=None, datefmt=None, style='%'):
        '''Match coloredlogs.ColoredFormatter arguments with logging.Formatter'''
        coloredlogs.ColoredFormatter.__init__(self, fmt=fmt, datefmt=datefmt)
        coloredlogs.DEFAULT_FIELD_STYLES = {
            "asctime": {
                "color": "green"
            },
            "hostname": {
                "color": "magenta"
            },
            "levelname": {
                "faint": True,
                "color": 214
            },
            "name": {
                "color": "blue"
            },
            "programname": {
                "color": "cyan"
            },
            "username": {
                "color": "yellow"
            }
        }
        coloredlogs.DEFAULT_LEVEL_STYLES = {
            "critical": {
                "bold": True,
                "color": "red"
            },
            "logging": {
                "color": "magenta"
            },
            "error": {
                "color": "red"
            },
            "info": {
                "color": "blue"
            },
            "notice": {
                "color": "magenta"
            },
            "spam": {
                "color": "green",
                "faint": True
            },
            "success": {
                "bold": True,
                "color": "green"
            },
            "verbose": {
                "color": "blue"
            },
            "warning": {
                "color": "yellow"
            }
        }

logging.config.fileConfig(fname="scripts/logger.ini", disable_existing_loggers=False)
LOGGER = logging.getLogger("AssetBundler")

ASSET_TYPES: dict[str, str] = {
    ".tim": "TEXTURE",
    ".smd": "MODEL"
}
MAX_BUNDLE_NAME_SIZE = 8
MAX_FILE_NAME_SIZE = 16

def createDefines(name: str, index: int, files: list[str]) -> list[str]:
    defines = [f"#define ASSET_BUNDLE__{name} {index}"]
    i = 0
    for f in files:
        f_path = Path(f)
        asset_type = ASSET_TYPES[f_path.suffix]
        f_name = f_path.name.removesuffix(f_path.suffix)
        defines.append(f"#define ASSET_{asset_type}__{name}__{f_name.upper()} {i}")
        i += 1
    return defines

def createXML(name: str, files: list[str]) -> ET.ElementTree:
    builder = ET.TreeBuilder()
    builder.start("lzp_project", {})
    builder.start(
        "create",
        {
            "packname": f"{name}.qlp",
            "format": "qlp"
        }
    )
    for file in files:
        builder.start(
            "file",
            {
                "alias": "inventory"
            }
        )
        builder.data(f"${{ASSETS_DIR}}/{name}/{file}")
        builder.end("file")
    builder.end("create")
    builder.start(
        "create",
        {
            "packname": f"{name}.lzp",
            "format": "lzp"
        }
    )
    builder.start(
        "file",
        {
            "alias": f"{name}"
        }
    )
    builder.data(f"{name}.qlp")
    builder.end("file")
    builder.end("create")
    return ET.ElementTree(builder.end("lzp_project"))

def packBundle(name: str, index: int, path: Path) -> Tuple[ET.ElementTree, list[str]]:
    files: list[str] = []
    for f in os.listdir(path):
        if not os.path.isfile(os.path.join(path, f)):
            continue
        f_path = Path(f)
        f_name = f_path.name.removesuffix(f_path.suffix)
        if len(f_name) > MAX_FILE_NAME_SIZE:
            LOGGER.error(f"File assets/{name}/{f_path.name} exceeds max name length: {len(f_name)} > {MAX_FILE_NAME_SIZE}, skipping")
            continue
        if f_path.suffix in ASSET_TYPES:
            LOGGER.info(f"Including {ASSET_TYPES[f_path.suffix]} asset: {f_path}")
            files.append(f_path.name)
        else:
            LOGGER.warning(f"Unknown asset type: {f_path.suffix}, skipping")
    files.sort()
    return createXML(name, files), createDefines(name.upper(), index, files)

def constructCMakeBindings(bundle_names: list[str]) -> None:
    bundle_names = [f"PackAssets({name} {name}.qlp)" for name in bundle_names]
    packing = "\n".join(bundle_names)
    content = f"""function(PackAssets bundle byproducts)
    set(ASSETS_DIR ${{PROJECT_SOURCE_DIR}}/assets)
    configure_file("assets/${{bundle}}.xml" "assets/_${{bundle}}.xml")
    file(GLOB_RECURSE _assets ${{ASSETS_DIR}}/${{bundle}}/*.tim ${{ASSETS_DIR}}/${{bundle}}/*.smd)
    list(LENGTH _assets asset_count)
    if (NOT (asset_count EQUAL 0))
        add_custom_command(
            COMMAND    ${{LZPACK}} -y "assets/_${{bundle}}.xml"
            OUTPUT     ${{bundle}}.lzp
            BYPRODUCTS ${{byproducts}}
            DEPENDS    ${{_assets}}
            COMMENT    "Building ${{bundle}} LZP archive"
        )
    endif()
    message("Globbed ${{asset_count}} ${{bundle}} asset(s):")
    foreach (_asset ${{_assets}})
        message("- ${{_asset}}")
    endforeach()
endfunction()

macro(ConstructAssetBundles)
    {packing}
endmacro()
""" 
    with open("CMakeLists.bundles.txt", "w") as file:
        file.write(content)
    
def main() -> None:
    defines = []
    index = 0
    bundle_names: list[str] = []
    for _ , dirs, _ in os.walk(u"assets"):
        for dir in dirs:
            if len(dir) > MAX_BUNDLE_NAME_SIZE:
                LOGGER.error(f"Bundle name {dir} exceeds maximum length of {MAX_BUNDLE_NAME_SIZE}, skipping")
                continue
            LOGGER.info(f"Bundling assets of {dir}")
            bundle_names.append(dir)
            bundle, bundle_defines = packBundle(dir, index, Path(f"./assets/{dir}").absolute())
            ET.indent(bundle)
            bundle.write(f"./assets/{dir}.xml")
            LOGGER.info(f"Wrriten {dir} assets to assets/{dir}.xml")
            defines.append(bundle_defines)
            index += 1
    defines = ["\n".join(bundle_defines) for bundle_defines in defines]
    defines_string = "\n\n".join(defines)
    defines_content = f"""#pragma once

#ifndef PSXMC__RESOURCES__ASSET_INDICES_H
#define PSXMC__RESOURCES__ASSET_INDICES_H

{defines_string}

#endif // PSXMC__RESOURCES__ASSET_INDICES_H
"""
    LOGGER.info("Writing defines to src/resources/asset_indices.h")
    with open("src/resources/asset_indices.h", "w") as file:
        file.write(defines_content)
    LOGGER.info("Constructing CMake bindings")
    constructCMakeBindings(bundle_names)

if __name__ == "__main__":
    main()
