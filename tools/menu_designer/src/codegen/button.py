import pygame
from src.preview import PreviewButton
from jinja2 import Environment, FileSystemLoader, select_autoescape
import caseconverter

env = Environment(
    loader = FileSystemLoader("templates"),
    autoescape = select_autoescape()
)

FONTS = [
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
]

def gen_button_code(button: PreviewButton) -> str:
    render_parameters = {
        "name_snake_lower": caseconverter.snakecase(button.label.text),
        "text": button.label.text,
        "x": button.get_pos()[0],
        "y": button.get_pos()[1],
        "width": button.get_width(),
    }
    return env.get_template("button.j2").render(render_parameters)

def find_font() -> str:
    for font in FONTS:
        try:
            print(f"Font: {font}")
            pygame.font.match_font(font)
            return font 
        except:
            continue
    return pygame.font.get_default_font()

def gen_button_html_code(button: PreviewButton) -> str:
    render_parameters = {
        "name_snake_lower": caseconverter.snakecase(button.label.text),
        "text": button.label.text,
        "x": button.get_pos()[0],
        "y": button.get_pos()[1],
        "width": button.get_width(),
        "font": find_font()
    }
    return env.get_template("button_html.j2").render(render_parameters)
