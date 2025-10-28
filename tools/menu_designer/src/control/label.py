import pygame_gui
from src.control.base import ControlBase

class ControlLabel(ControlBase):
    label: pygame_gui.elements.UILabel

    def __init__(
        self,
        x: int,
        y: int,
        width: int,
        height: int,
        text: str,
        manager: pygame_gui.UIManager,
        control_container: pygame_gui.core.IContainerLikeInterface
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
