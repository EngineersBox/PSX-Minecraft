import pygame, pygame_gui
from src.control.base import ControlBase
from typing import Callable

type ControlWindowClosedCommand = Callable[[], None]
type ControlWindowResizedCommand = Callable[[int, int], None]

def __default_resized_command(width: int, height: int):
    pass

class ControlWindow(ControlBase):
    window: pygame_gui.elements.UIWindow
    closed_command: ControlWindowClosedCommand
    resized_command: ControlWindowResizedCommand

    def __init__(
        self,
        x: int,
        y: int,
        width: int,
        height: int,
        title: str,
        closed_command: ControlWindowClosedCommand,
        manager: pygame_gui.UIManager,
        resized_command: ControlWindowResizedCommand = __default_resized_command,
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

    def process_event(self, event: pygame.Event):
        if (event.type == pygame_gui.UI_WINDOW_CLOSE
            and event.ui_element == self.window):
            self.closed_command()
        elif (self.window.resizable
            and event.type == pygame_gui.UI_WINDOW_RESIZED
            and event.ui_element == self.window):
            abs_rect = self.window.get_abs_rect()
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

