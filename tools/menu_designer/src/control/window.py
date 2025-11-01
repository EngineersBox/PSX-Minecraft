import pygame, pygame_gui
from src.control.base import ControlBase
from typing import Callable, Optional, TypeVar

T = TypeVar("T", bound="ControlBase")

type ControlWindowClosedCommand = Callable[[], None]
type ControlWindowResizedCommand = Callable[[int, int], None]

class ControlWindow(ControlBase):
    window: pygame_gui.elements.UIWindow
    closed_command: ControlWindowClosedCommand
    resized_command: Optional[ControlWindowResizedCommand]

    _update_elements: dict[str, ControlBase]
    _process_event_elements: dict[str, ControlBase]

    def __init__(
        self,
        x: int,
        y: int,
        width: int,
        height: int,
        title: str,
        closed_command: ControlWindowClosedCommand,
        manager: pygame_gui.UIManager,
        resized_command: Optional[ControlWindowResizedCommand] = None,
        draggable: bool = False,
        resizable: bool = False,
        always_on_top: bool = False
    ):
        super().__init__(
            x,
            y,
            width,
            height
        )
        self.window = pygame_gui.elements.UIWindow(
            rect=self.rect,
            window_display_title=title,
            manager=manager,
            draggable=draggable,
            resizable=resizable,
            always_on_top=always_on_top
        )
        self.closed_command = closed_command
        self.resized_command = resized_command
        self._update_elements = {}
        self._process_event_elements = {}

    def kill(self):
        self._update_elements.clear()
        self._process_event_elements.clear()
        self.window.kill()

    def add_element(
        self,
        element: T,
        update: bool = False,
        process_event: bool = False
    ) -> T:
        if update:
            self._update_elements[element.element_id] = element
        if process_event:
            self._process_event_elements[element.element_id] = element
        return element

    def update(self, time_delta: float):
        for element in self._update_elements.values():
            element.update(time_delta)

    def process_event(self, event: pygame.Event):
        for element in self._process_event_elements.values():
            element.process_event(event)
        if (event.type == pygame_gui.UI_WINDOW_CLOSE
            and event.ui_element == self.window):
            self.closed_command()
        elif (self.window.resizable
            and event.type == pygame_gui.UI_WINDOW_RESIZED
            and event.ui_element == self.window):
            abs_rect = self.window.get_abs_rect()
            if self.resized_command != None:
                self.resized_command(
                    int(abs_rect.width),
                    int(abs_rect.height)
                )

    def hide(self):
        self.window.disable()
        self.window.hide()

    def show(self):
        self.window.enable()
        self.window.show()

    def disable(self):
        self.window.disable()

    def enable(self):
        self.window.enable()

