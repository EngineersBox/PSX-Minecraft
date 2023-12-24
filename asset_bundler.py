import xml.etree.ElementTree as ET
import os, pathlib


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


def bundle(lzp_project):
    assets = ET.SubElement(
        lzp_project,
        "create",
        {
            "packname": "assets.lzp",
            "format": "lzp"
        }
    )
    for root, dirs, files in os.walk(u"assets"):
        if len(files) == 0:
            continue
        for file in files:
            file_path = pathlib.Path(root + "/" + file)
            if file_path.suffix not in ASSET_TYPES:
                continue
            root.removeprefix("assets")
            bind_element(
                lzp_project,
                assets,
                ASSET_TYPES[file_path.suffix],
                file_path.name.removesuffix(file_path.suffix),
                "${ASSETS_DIR}" + root.removeprefix("assets") + "/" + file
            )

    pass

def sort_elements(lzp_project):
    assets = lzp_project.find("create[@packname='assets.lzp']")
    lzp_project.remove(assets)
    lzp_project.append(assets)

def main():
    with open("assets.xml", "w+") as file:
        if os.path.getsize("assets.xml") == 0:
            file.write("<lzp_project></lzp_project>")
    lzp_project = ET.parse("assets.xml")
    bundle(lzp_project.getroot())
    sort_elements(lzp_project.getroot())
    ET.indent(lzp_project)
    lzp_project.write("assets.xml")


if __name__ == "__main__":
    main()
    # pass
