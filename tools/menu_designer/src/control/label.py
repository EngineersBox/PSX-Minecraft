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

    def set_text(self, text: str):
        self.label.set_text(text)

    def disable(self):
        self.label.disable()

    def enable(self):
        self.label.enable()

    def hide(self):
        self.label.hide()

    def show(self):
        self.label.show()
