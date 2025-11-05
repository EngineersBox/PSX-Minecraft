from typing import Optional
import pygame
import pygame_gui
from src.control.base import ControlBase

class ControlLabel(ControlBase):
    label: pygame_gui.elements.UILabel

    _bg_surface: Optional[pygame.Surface]

    def __init__(
        self,
        x: int,
        y: int,
        width: int,
        height: int,
        text: str,
        manager: pygame_gui.UIManager,
        control_container: pygame_gui.core.IContainerLikeInterface,
        background_surface: Optional[pygame.Surface] = None
    ):
        super().__init__(
            x,
            y,
            width,
            height
        )
        self.label = pygame_gui.elements.UILabel(
            relative_rect=self.rect,
            text=text,
            manager=manager,
            container=control_container
        )
        self._bg_surface = background_surface

    def kill(self):
        self.label.kill()

    def set_text(self, text: str):
        self.label.set_text(text)

    def get_text(self) -> str:
        return self.label.text

    def disable(self):
        self.label.disable()

    def enable(self):
        self.label.enable()

    def hide(self):
        self.label.hide()

    def show(self):
        self.label.show()

    def set_background_colour(self, colour: pygame.Color):
        if self._bg_surface == None:
            self._bg_surface = pygame.Surface((
                self.rect.width,
                self.rect.height
            ))
        self._bg_surface.fill(colour)
        self.label.set_image(self._bg_surface)

    def clear_background_colour(self):
        self.label.set_image(None)
        self._bg_surface = None
