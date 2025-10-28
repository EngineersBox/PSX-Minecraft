import pygame_gui
from typing import Callable
from src.control.base import ControlBase

class ControlButton(ControlBase):
    button: pygame_gui.elements.UIButton

    def __init__(
        self,
        x: int,
        y: int,
        width: int,
        height: int,
        text: str,
        manager: pygame_gui.UIManager,
        control_container: pygame_gui.core.IContainerLikeInterface,
        command: Callable
    ):
        super().__init__(
            x,
            y,
            width,
            height
        )
        self.button = pygame_gui.elements.UIButton(
            relative_rect=self.rect,
            text=text,
            manager=manager,
            container=control_container,
            command=command
        )

    def enable(self):
        self.button.enable()

    def disable(self):
        self.button.disable()

    def kill(self):
        self.button.kill()
