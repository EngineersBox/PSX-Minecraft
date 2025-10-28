from src.preview import PreviewButton
from jinja2 import Environment, FileSystemLoader, select_autoescape
import caseconverter

env = Environment(
    loader = FileSystemLoader("templates"),
    autoescape = select_autoescape()
)

def gen_button_code(button: PreviewButton) -> str:
    render_parameters = {
        "name_snake_lower": caseconverter.snakecase(button.label.text),
        "text": button.label.text,
        "x": button.get_pos()[0],
        "y": button.get_pos()[1],
        "width": button.get_width(),
    }
    return env.get_template("button.j2").render(render_parameters)

def gen_button_html_code(button: PreviewButton) -> str:
    render_parameters = {
        "name_snake_lower": caseconverter.snakecase(button.label.text),
        "text": button.label.text,
        "x": button.get_pos()[0],
        "y": button.get_pos()[1],
        "width": button.get_width(),
    }
    return env.get_template("button_html.j2").render(render_parameters)
