from typing import Callable, Optional
import pygame, pygame_gui
from src.control.base import ControlBase

type ControlCheckboxCommand = Callable[[bool], None]

class ControlCheckbox(ControlBase):
    checkbox: pygame_gui.elements.UICheckBox
    command: Optional[ControlCheckboxCommand]

    def __init__(
        self,
        x: int,
        y: int,
        width: int,
        height: int,
        text: str,
        manager: pygame_gui.UIManager,
        container: pygame_gui.core.IContainerLikeInterface,
        command: Optional[ControlCheckboxCommand] = None
    ):
        super().__init__(
            x,
            y,
            width,
            height
        )
        self.checkbox = pygame_gui.elements.UICheckBox(
            relative_rect=self.rect,
            text=text,
            manager=manager,
            container=container
        )
        self.command = command

    def process_event(self, event: pygame.Event):
        if self.command == None:
            return
        if (event.type == pygame_gui.UI_CHECK_BOX_CHECKED
            and event.ui_element == self.checkbox):
            self.command(True)
        elif (event.type == pygame_gui.UI_CHECK_BOX_UNCHECKED
            and event.ui_element == self.checkbox):
            self.command(False)

    def set_state(self, state: bool):
        self.checkbox.set_state(state)

    def get_state(self) -> bool:
        state = self.checkbox.get_state()
        if type(state) == bool:
            return state
        return False

    def enable(self):
        self.checkbox.enable()

    def disable(self):
        self.checkbox.disable()

    def hide(self):
        self.checkbox.hide()

    def show(self):
        self.checkbox.show()

    def kill(self):
        self.checkbox.kill()
