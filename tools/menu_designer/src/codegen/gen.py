from typing import Optional, cast
import pygame, caseconverter
from src.preview.background import PreviewBackground
from src.preview.element import PreviewElement
from src.preview.button import PreviewButton
from jinja2 import Environment, FileSystemLoader, select_autoescape

env = Environment(
    loader = FileSystemLoader("templates"),
    autoescape = select_autoescape()
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

def gen_button_code(button: PreviewButton, font: str) -> str:
    render_parameters = {
        "name_snake_lower": caseconverter.snakecase(button.label.text),
        "text": button.label.text,
        "x": button.get_pos()[0],
        "y": button.get_pos()[1],
        "width": button.get_width(),
        "disabled": button.disabled,
        "font": font
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

def gen_background_html_code(background: PreviewBackground, font: str) -> str:
    tint = background.get_tint()
    render_parameters = {
        "name_snake_lower": caseconverter.snakecase(background.get_name()),
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

def gen_html_code(element: PreviewElement, font_name: Optional[str] = None) -> str:
    font = font_name if font_name is not None else find_font()
    element_type = type(element)
    if element_type == PreviewButton:
        return gen_button_html_code(cast(PreviewButton, element), font)
    elif element_type == PreviewBackground:
        return gen_background_html_code(cast(PreviewBackground, element), font)
    raise ValueError(f"Unknown element type: {element_type}")
