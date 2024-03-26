import xml.etree.ElementTree as ET
import os, pathlib, logging, logging.config, coloredlogs

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
            "debug": {
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

logging.config.fileConfig(fname="logger.ini", disable_existing_loggers=False)
LOGGER = logging.getLogger("AssetBundler")

MAX_FILE_NAME_SIZE = 16


def create_element(parent, name, packname):
    return ET.SubElement(
        parent,
        name,
        {
            "packname": packname + ".qlp",
            "format": "qlp"
        }
    )


def bind_element(lzp_project, assets, element_name, file, path):
    elements = lzp_project.find("create[@packname='" + element_name + ".qlp']")
    if elements is None:
        elements = ET.SubElement(
            lzp_project,
            "create",
            {
                "packname": element_name + ".qlp",
                "format": "qlp"
            }
        )
    assets_binding = assets.find("file[@alias='" + element_name + "']")
    if assets_binding is None:
        assets_binding = ET.SubElement(
            assets,
            "file",
            {
                "alias": element_name
            }
        )
        assets_binding.text = element_name + ".qlp"
    file_element = ET.SubElement(
        elements,
        "file",
        {
            "alias": file
        }
    )
    file_element.text = path


ASSET_TYPES = {
    ".tim": "textures",
    ".smd": "models"
}

def bundle(lzp_project) -> (int, int, int):
    assets = ET.SubElement(
        lzp_project,
        "create",
        {
            "packname": "assets.lzp",
            "format": "lzp"
        }
    )
    success = 0
    fail = 0
    skipped = 0
    for root, dirs, files in os.walk(u"assets"):
        if len(files) == 0:
            continue
        for file in files:
            try:
                file_path = pathlib.Path(root + "/" + file)
                if file_path.suffix not in ASSET_TYPES:
                    LOGGER.warning(f"Unknown asset type {file_path.suffix}, skipping")
                    skipped += 1
                    continue
                file_name = file_path.name.removesuffix(file_path.suffix)
                if len(file_name) > MAX_FILE_NAME_SIZE:
                    LOGGER.error(f"File {file_name} exceeds max name length: {len(file_name)} > {MAX_FILE_NAME_SIZE}")
                    fail += 1
                    continue
                if len(file_path.parts) > 3:
                    LOGGER.warning(f"File {file_name} is nested more than one level, skipping unsupported file")
                    skipped += 1
                    continue
                LOGGER.info(f"Bundling asset {file_path}")
                element_name = None
                if len(file_path.parts) > 2:
                    element_name = file_path.parts[1]
                else:
                    element_name = ASSET_TYPES[file_path.suffix]
                    LOGGER.info(f"Asset {file_name} has no namespacing, deriving from asset extension: '{file_path.suffix}' => '{element_name}'")
                bind_element(
                    lzp_project,
                    assets,
                    element_name,
                    file_path.name.removesuffix(file_path.suffix),
                    "${ASSETS_DIR}" + root.removeprefix("assets") + "/" + file
                )
                success += 1
            except Exception as e:
                fail += 1
                LOGGER.error(e)
    return success, fail, skipped


def sort_elements(lzp_project: ET.Element):
    assets = lzp_project.find("create[@packname='assets.lzp']")
    lzp_project.remove(assets)
    lzp_project.append(assets)


def generate_defines(lzp_project: ET.Element) -> str:
    defines = ""
    pack_index = 0
    for element in lzp_project.findall("create[@format='qlp']"):
        define_prefix = element.attrib["packname"].removesuffix(".qlp")
        defines += "#define " + f"ASSET_{define_prefix}_INDEX {pack_index}".upper() + "\n"
        pack_asset_index = 0
        for file_element in element:
            defines += "#define " + f"ASSET_{define_prefix}_{file_element.attrib['alias']}_INDEX {pack_asset_index}".upper() + "\n"
            pack_asset_index += 1
        pack_index += 1
    return """#pragma once

#ifndef PSX_MINECRAFT_ASSET_INDICES_H
#define PSX_MINECRAFT_ASSET_INDICES_H

""" + defines + """
#endif // PSX_MINECRAFT_ASSET_INDICES_H    
"""

def main():
    with open("assets.xml", "w+") as file:
        if os.path.getsize("assets.xml") == 0:
            file.write("<lzp_project></lzp_project>")
    lzp_project = ET.parse("assets.xml")
    success, fail, skipped = bundle(lzp_project.getroot())
    sort_elements(lzp_project.getroot())
    defines = generate_defines(lzp_project.getroot())
    with open("src/resources/asset_indices.h", "w") as file:
        file.write(defines)
    ET.indent(lzp_project)
    lzp_project.write("assets.xml")
    LOGGER.info("Written bundled assets XML to assets.xml")
    LOGGER.info(f"Summary: [Success: {success}] [Failed: {fail}] [Skipped: {skipped}]")


if __name__ == "__main__":
    # main()
    print("Assets bundled manually, ignoring.")
