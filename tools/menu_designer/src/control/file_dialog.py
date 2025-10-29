import pygame, pygame_gui
from pathlib import Path
from typing import Callable, Optional
from src.control.base import ControlBase

type ControlFileDialogSelectedCommand = Callable[[Path], None]
type ControlFileDialogClosedCommand = Callable[[], None]

class ControlFileDialog(ControlBase):
    file_dialog: pygame_gui.windows.UIFileDialog
    selected_command: ControlFileDialogSelectedCommand
    closed_command: ControlFileDialogClosedCommand

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
        initial_file_path: Optional[str] = None,
        allow_existing_files_only: bool = True,
        allow_picking_directories: bool = False
    ):
        super().__init__(
            x,
            y,
            width,
            height
        )
        self.file_dialog = pygame_gui.windows.UIFileDialog(
            rect=self.rect,
            manager=manager,
            window_title=title,
            initial_file_path=initial_file_path,
            allow_existing_files_only=allow_existing_files_only,
            allow_picking_directories=allow_picking_directories,
            always_on_top=True
        )
        self.file_dialog.disable()
        self.file_dialog.hide()
        self.selected_command = selected_command
        self.closed_command = closed_command

    def open(self):
        self.file_dialog.enable()
        self.file_dialog.show()

    def process_event(self, event: pygame.Event):
        if (event.type != pygame_gui.UI_FILE_DIALOG_PATH_PICKED
            or event.ui_element != self.file_dialog):
            return
        path = self.file_dialog.current_file_path
        if path == None:
            self.closed_command()
        else:
            self.selected_command(path)

