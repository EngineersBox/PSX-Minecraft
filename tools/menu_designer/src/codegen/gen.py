from typing import Optional, cast
import pygame, caseconverter
from src.preview.background import PreviewBackground
from src.preview.element import PreviewElement
from src.preview.button import PreviewButton
from jinja2 import Environment, FileSystemLoader, select_autoescape

env = Environment(
    loader = FileSystemLoader("templates"),
    autoescape = select_autoescape(),
    trim_blocks=True,
    lstrip_blocks=False
)

FONTS = {
    "monocraft",
    "jetbrainsmono",
    "firacode",
    "firamono",
    "sfmono",
    "sourcecodepro",
    "cascadiacode",
    "menlo",
    "monaco",
    "consolas",
    "courier"
}

def gen_button_code(button: PreviewButton) -> str:
    render_parameters = {
        "name_snake_lower": caseconverter.snakecase(button.label.text),
        "text": button.label.text,
        "x": button.get_pos()[0],
        "y": button.get_pos()[1],
        "width": button.get_width(),
        "disabled": button.disabled
    }
    return env.get_template("button.j2").render(render_parameters)

def find_font() -> str:
    for font in FONTS:
        try:
            pygame.font.match_font(font)
            return font 
        except:
            continue
    return pygame.font.get_default_font()

def gen_button_html_code(button: PreviewButton, font: str) -> str:
    render_parameters = {
        "name_snake_lower": caseconverter.snakecase(button.label.text),
        "text": button.label.text,
        "x": button.get_pos()[0],
        "y": button.get_pos()[1],
        "width": button.get_width(),
        "disabled": button.disabled,
        "font": font
    }
    return env.get_template("button_html.j2").render(render_parameters)

def gen_background_code(background: PreviewBackground) -> str:
    tint = background.get_tint()
    render_parameters = {
        "name_snake_lower": caseconverter.snakecase(background.get_name()),
        "texture": background.get_tim_name(),
        "non_static_texture_name": caseconverter.snakecase(background.get_tim_name()) + "_texture",
        "texture_is_static": background.get_bundle_name() == "ASSET_BUNDLE__STATIC",
        "x": background.get_pos()[0],
        "y": background.get_pos()[1],
        "width": background.get_width(),
        "height": background.get_height(),
        "texture_u": background.get_pos_u(),
        "texture_v": background.get_pos_v(),
        "texture_width": background.get_tile_x(),
        "texture_height": background.get_tile_y(),
        "tint_r": tint.r,
        "tint_g": tint.g,
        "tint_b": tint.b
    }
    return env.get_template("background.j2").render(render_parameters)

def gen_background_html_code(background: PreviewBackground, font: str) -> str:
    tint = background.get_tint()
    render_parameters = {
        "name_snake_lower": caseconverter.snakecase(background.get_name()),
        "texture": background.get_tim_name(),
        "non_static_texture_name": caseconverter.snakecase(background.get_tim_name()) + "_texture",
        "texture_is_static": background.get_bundle_name() == "ASSET_BUNDLE__STATIC",
        "x": background.get_pos()[0],
        "y": background.get_pos()[1],
        "width": background.get_width(),
        "height": background.get_height(),
        "texture_u": background.get_pos_u(),
        "texture_v": background.get_pos_v(),
        "texture_width": background.get_tile_x(),
        "texture_height": background.get_tile_y(),
        "tint_r": tint.r,
        "tint_g": tint.g,
        "tint_b": tint.b,
        "font": font
    }
    return env.get_template("background_html.j2").render(render_parameters)

def gen_code(element: PreviewElement) -> str:
    element_type = type(element)
    if element_type == PreviewButton:
        return gen_button_code(cast(PreviewButton, element))
    elif element_type == PreviewBackground:
        return gen_background_code(cast(PreviewBackground, element))
    raise ValueError(f"Unknown element type: {element_type}")

def gen_html_code(element: PreviewElement, font_name: Optional[str] = None) -> str:
    font = font_name if font_name is not None else find_font()
    element_type = type(element)
    if element_type == PreviewButton:
        return gen_button_html_code(cast(PreviewButton, element), font)
    elif element_type == PreviewBackground:
        return gen_background_html_code(cast(PreviewBackground, element), font)
    raise ValueError(f"Unknown element type: {element_type}")

def gen_menu_header_code(name: str) -> str:
    render_parameters = {
        "name_upper_snake_case": caseconverter.macrocase(name),
        "name_camel_case": caseconverter.camelcase(name),
        "name_pascal_case": caseconverter.pascalcase(name)
    }
    return env.get_template("menu.h.j2").render(render_parameters)

def gen_menu_header_html_code(name: str, font: str) -> str:
    render_parameters = {
        "name_upper_snake_case": caseconverter.macrocase(name),
        "name_camel_case": caseconverter.camelcase(name),
        "name_pascal_case": caseconverter.pascalcase(name),
        "font": font
    }
    return env.get_template("menu.h_html.j2").render(render_parameters)

def gen_menu_source_code(name: str, elements: list[PreviewElement]) -> str:
    static_textures: dict[str, tuple[str, str, str]] = {}
    for element in elements:
        if type(element) != PreviewBackground:
            continue
        background = cast(PreviewBackground, element)
        if background.get_bundle_name() == "ASSET_BUNDLE__STATIC":
            continue
        non_static_texture_name = caseconverter.snakecase(background.get_tim_name()) + "_texture"
        if non_static_texture_name in static_textures:
            continue
        static_textures[non_static_texture_name] = (
            background.get_bundle_name(),
            background.get_tim_name(),
            non_static_texture_name
        )
    render_parameters = {
        "name_snake_case": caseconverter.snakecase(name),
        "name_upper_snake_case": caseconverter.macrocase(name),
        "name_camel_case": caseconverter.camelcase(name),
        "name_pascal_case": caseconverter.pascalcase(name),
        "components_init_code": "    " + "\n".join(map(gen_code, elements)).replace("\n", "\n    "),
        "static_textures": list(static_textures.values())
    }
    return env.get_template("menu.c.j2").render(render_parameters)

def gen_menu_source_html_code(name: str, elements: list[PreviewElement], font: str) -> str:
    static_textures: dict[str, tuple[str, str, str]] = {}
    for element in elements:
        if type(element) != PreviewBackground:
            continue
        background = cast(PreviewBackground, element)
        if background.get_bundle_name() == "ASSET_BUNDLE__STATIC":
            continue
        non_static_texture_name = caseconverter.snakecase(background.get_tim_name()) + "_texture"
        if non_static_texture_name in static_textures:
            continue
        static_textures[non_static_texture_name] = (
            background.get_bundle_name(),
            background.get_tim_name(),
            non_static_texture_name
        )
    render_parameters = {
        "name_snake_case": caseconverter.snakecase(name),
        "name_upper_snake_case": caseconverter.macrocase(name),
        "name_camel_case": caseconverter.camelcase(name),
        "name_pascal_case": caseconverter.pascalcase(name),
        "components_init_code": "    " + "\n".join(map(gen_html_code, elements)).replace("\n", "\n    "),
        "static_textures": list(static_textures.values()),
        "font": font
    }
    return env.get_template("menu.c_html.j2").render(render_parameters)
