import pygame_gui
from src.control.base import ControlBase

class ControlTextInput(ControlBase):
    input: pygame_gui.elements.UITextEntryLine

    def __init__(
        self,
        x: int,
        y: int,
        width: int,
        height: int,
        manager: pygame_gui.UIManager,
        control_container: pygame_gui.core.IContainerLikeInterface
    ):
        super().__init__(
            x,
            y,
            width,
            height
        )
        self.input = pygame_gui.elements.UITextEntryLine(
            relative_rect=self.rect,
            placeholder_text="Button text...",
            manager=manager,
            container=control_container
        )

