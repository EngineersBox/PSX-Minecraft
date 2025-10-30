import pygame, pygame_gui
from typing import Optional
from src.control.base import ControlBase

class ControlTextBox(ControlBase):
    surface: pygame.Surface
    text_box: pygame_gui.elements.UITextBox

    def __init__(
        self,
        x: int,
        y: int,
        width: int,
        height: int,
        manager: pygame_gui.UIManager,
        container: Optional[pygame_gui.core.IContainerLikeInterface] = None,
        background_colour: Optional[pygame.Color] = None
    ):
        super().__init__(
            x,
            y,
            width,
            height
        )
        self.text_box = pygame_gui.elements.UITextBox(
            html_text="",
            relative_rect=self.rect,
            manager=manager,
            container=container,
            allow_split_dashes=False
        )
        self.text_box.line_spacing = 1
        if background_colour is not None:
            self.surface = pygame.Surface((width, height))
            self.surface.fill(background_colour)
            self.text_box.set_image(self.surface)
            self.text_box.background_colour = background_colour

    def set_text(self, text: str):
        self.text_box.set_text(text)

    def kill(self):
        self.text_box.kill()

    def disable(self):
        self.text_box.disable()

    def enable(self):
        self.text_box.enable()

    def hide(self):
        self.text_box.hide()

    def show(self):
        self.text_box.show()
