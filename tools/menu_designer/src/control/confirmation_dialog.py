import pygame, pygame_gui
from typing import Callable
from src.control.base import ControlBase

type ControlConfirmationDialogConfirmCommand = Callable[[], None]
type ControlConfirmationDialogClosedCommand = Callable[[], None]

class ControlConfirmationDialog(ControlBase):
    dialog: pygame_gui.windows.UIConfirmationDialog
    confirm_command: ControlConfirmationDialogConfirmCommand
    closed_command: ControlConfirmationDialogClosedCommand

    def __init__(
        self,
        x: int,
        y: int,
        width: int,
        height: int,
        title: str,
        description: str,
        confirm_command: ControlConfirmationDialogConfirmCommand,
        closed_command: ControlConfirmationDialogClosedCommand,
        manager: pygame_gui.UIManager
    ):
        super().__init__(
            x,
            y,
            width,
            height
        )
        self.dialog = pygame_gui.windows.UIConfirmationDialog(
            rect=self.rect,
            action_long_desc=description,
            window_title=title,
            manager=manager,
            always_on_top=True
        )
        self.confirm_command = confirm_command
        self.closed_command = closed_command

    def process_event(self, event: pygame.Event):
        if (event.type == pygame_gui.UI_CONFIRMATION_DIALOG_CONFIRMED
            and event.ui_element == self.dialog):
            self.confirm_command()
        elif (event.type == pygame_gui.UI_WINDOW_CLOSE
            and event.ui_element == self.dialog):
            self.closed_command()
        elif (event.type == pygame_gui.UI_BUTTON_PRESSED
            and event.ui_element == self.dialog.cancel_button):
            self.closed_command()

    def disable(self):
        self.dialog.disable()

    def enable(self):
        self.dialog.enable()

    def hide(self):
        self.dialog.hide()

    def show(self):
        self.dialog.show()

    def kill(self):
        self.dialog.kill()
