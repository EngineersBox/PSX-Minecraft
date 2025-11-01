import re
from typing import Callable, Optional
import pygame, pygame_gui
from src.control.base import ControlBase

type ControlTextInputValidateCharacter = Callable[[str], bool]

NUMBERS_ONLY_REGEX = re.compile(r"^\d$")

def number_only_validator(char: str) -> bool:
    if len(char) != 1:
        return False
    match = NUMBERS_ONLY_REGEX.fullmatch(char)
    return match != None and type(match) == re.Match

class ControlTextInput(ControlBase):
    input: pygame_gui.elements.UITextEntryLine

    _validator_command: Optional[ControlTextInputValidateCharacter]

    def __init__(
        self,
        x: int,
        y: int,
        width: int,
        height: int,
        manager: pygame_gui.UIManager,
        control_container: pygame_gui.core.IContainerLikeInterface,
        validator_command: Optional[ControlTextInputValidateCharacter] = None,
        initial_text: Optional[str] = None,
        placeholder_text: Optional[str] = None
    ):
        super().__init__(
            x,
            y,
            width,
            height
        )
        self.input = pygame_gui.elements.UITextEntryLine(
            relative_rect=self.rect,
            initial_text=initial_text,
            placeholder_text=placeholder_text,
            manager=manager,
            container=control_container
        )
        self._validator_command = validator_command

    def process_event(self, event: pygame.Event):
        if self._validator_command == None:
            return
        if (event.type == pygame_gui.UI_TEXT_ENTRY_CHANGED
            and event.ui_element == self.input):
            text = self.input.get_text()
            if len(text) == 0:
                return
            if self._validator_command(text[-1:]):
                return
            text = text[:-1]
            self.input.set_text(text)

    def kill(self):
        self.input.kill()

    def get_text(self) -> str:
        return self.input.get_text()

    def disable(self):
        self.input.disable()

    def enable(self):
        self.input.enable()

    def hide(self):
        self.input.hide()

    def show(self):
        self.input.show()
