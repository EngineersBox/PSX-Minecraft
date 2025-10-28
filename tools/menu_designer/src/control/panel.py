from typing import Optional
import pygame
import pygame_gui
from src.control.base import ControlBase

class ControlPanel(ControlBase):
    surface: pygame.Surface
    panel: pygame_gui.elements.UIPanel

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
        self.panel = pygame_gui.elements.UIPanel(
            relative_rect=self.rect,
            manager=manager,
            container=container
        )
        if background_colour is not None:
            self.surface = pygame.Surface((width, height))
            self.surface.fill(background_colour)
            self.panel.set_background_images([self.surface])

    def show(self):
        self.panel.show()

    def hide(self):
        self.panel.hide()

    def kill(self):
        self.panel.kill()
