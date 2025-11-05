from typing import Callable, Optional
import pygame, pygame_gui
from src.control.base import ControlBase

type ControlColourPickerSelectedCommand = Callable[[pygame.Color], None]
type ControlColourPickerClosedCommand = Callable[[], None]

class ControlColourPicker(ControlBase):
    colour_picker: Optional[pygame_gui.windows.UIColourPickerDialog]
    
    _manager: pygame_gui.UIManager
    _title: str
    _selected_command: ControlColourPickerSelectedCommand
    _closed_command: ControlColourPickerClosedCommand

    def __init__(
        self,
        x: int,
        y: int,
        width: int,
        height: int,
        title: str,
        selected_command: ControlColourPickerSelectedCommand,
        closed_command: ControlColourPickerClosedCommand,
        manager: pygame_gui.UIManager
    ):
        super().__init__(
            x,
            y,
            width,
            height
        )
        self.colour_picker = None
        self._manager = manager
        self._title = title
        self._selected_command = selected_command
        self._closed_command = closed_command

    def update(self, time_delta: float):
        if self.colour_picker == None:
            return
        self.colour_picker.update(time_delta)

    def process_event(self, event: pygame.Event):
        if self.colour_picker == None:
            return
        if (event.type == pygame_gui.UI_COLOUR_PICKER_COLOUR_PICKED
            and event.ui_element == self.colour_picker):
            self._selected_command(self.colour_picker.get_colour())
        elif (event.type == pygame_gui.UI_BUTTON_PRESSED
            and event.ui_element == self.colour_picker.close_window_button):
            self._closed_command()
        elif (event.type == pygame_gui.UI_BUTTON_PRESSED
            and event.ui_element == self.colour_picker.cancel_button):
            self._closed_command()

    def open(self):
        self.colour_picker = pygame_gui.windows.UIColourPickerDialog(
            rect=self.rect,
            manager=self._manager,
            window_title=self._title,
            initial_colour=pygame.Color("#7F7F7F")
        )
        self.colour_picker.enable_close_button = True

    def close(self):
        if self.colour_picker == None:
            return
        self.colour_picker.kill()
        self.colour_picker = None

    def get_colour(self) -> pygame.Color:
        if self.colour_picker == None:
            return pygame.Color("#7F7F7F")
        return self.colour_picker.get_colour()

    def set_colour(self, colour: pygame.Color): 
        if self.colour_picker == None:
            return
        self.colour_picker.set_colour(colour)

    def disable(self):
        if self.colour_picker == None:
            return
        self.colour_picker.disable()

    def hide(self):
        if self.colour_picker == None:
            return
        self.colour_picker.hide()

    def enable(self):
        if self.colour_picker == None:
            return
        self.colour_picker.enable()

    def show(self):
        if self.colour_picker == None:
            return
        self.colour_picker.show()

    def kill(self):
        if self.colour_picker == None:
            return
        self.colour_picker.kill()
