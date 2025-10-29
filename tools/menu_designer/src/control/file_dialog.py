import pygame, pygame_gui
from pathlib import Path
from typing import Callable, Optional
from src.control.base import ControlBase

type ControlFileDialogSelectedCommand = Callable[[Path], None]
type ControlFileDialogClosedCommand = Callable[[], None]

class ControlFileDialog(ControlBase):
    file_dialog: Optional[pygame_gui.windows.UIFileDialog]
    selected_command: ControlFileDialogSelectedCommand
    closed_command: ControlFileDialogClosedCommand

    _manager: pygame_gui.UIManager
    _title: str

    def __init__(
        self,
        x: int,
        y: int,
        width: int,
        height: int,
        title: str,
        selected_command: ControlFileDialogSelectedCommand,
        closed_command: ControlFileDialogClosedCommand,
        manager: pygame_gui.UIManager,
    ):
        super().__init__(
            x,
            y,
            width,
            height
        )
        self.file_dialog = None
        self._manager = manager
        self._title = title
        self.selected_command = selected_command
        self.closed_command = closed_command

    def open(self):
        self.file_dialog = pygame_gui.windows.UIFileDialog(
            rect=self.rect,
            manager=self._manager,
            window_title=self._title,
            allow_existing_files_only=True,
            allow_picking_directories=False,
            always_on_top=True
        )
        self.file_dialog.enable()
        self.file_dialog.show()

    def process_event(self, event: pygame.Event):
        if (self.file_dialog == None):
            return
        if (event.type == pygame_gui.UI_BUTTON_PRESSED
            and event.ui_element == self.file_dialog.cancel_button):
            self.closed_command()
            self.file_dialog.hide()
            self.file_dialog.disable()
            self.file_dialog = None
            return
        if (event.type == pygame_gui.UI_BUTTON_PRESSED
            and event.ui_element == self.file_dialog.close_window_button):
            self.closed_command()
            self.file_dialog.hide()
            self.file_dialog.disable()
            self.file_dialog = None
            return
        if (event.type != pygame_gui.UI_FILE_DIALOG_PATH_PICKED
            or event.ui_element != self.file_dialog):
            return
        path = self.file_dialog.current_file_path
        if path == None:
            self.closed_command()
            self.file_dialog.hide()
            self.file_dialog.disable()
            self.file_dialog = None
        else:
            self.selected_command(path)

