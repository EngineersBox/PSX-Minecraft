import pygame_gui
from src.control.base import ControlBase

class ControlPanel(ControlBase):
    panel: pygame_gui.elements.UIPanel

    def __init__(
        self,
        x: int,
        y: int,
        width: int,
        height: int,
        manager: pygame_gui.UIManager,
        container: pygame_gui.core.IContainerLikeInterface | None = None
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
