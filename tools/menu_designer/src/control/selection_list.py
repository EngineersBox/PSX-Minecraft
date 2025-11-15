from typing import cast, Callable, Optional
import pygame, pygame_gui
from src.control.base import ControlBase

type ControlSelectionListSelectedCommand = Callable[[Optional[tuple[str, str]]], None]

class ControlSelectionList(ControlBase):
    selection_list: pygame_gui.elements.UISelectionList
    _selected_command: ControlSelectionListSelectedCommand

    _current_selection: Optional[tuple[str, str]]

    def __init__(
        self,
        x: int,
        y: int,
        width: int,
        height: int,
        item_list: list[tuple[str, str]],
        selected_command: ControlSelectionListSelectedCommand,
        manager: pygame_gui.UIManager,
        control_container: pygame_gui.core.IContainerLikeInterface
    ):
        super().__init__(x, y, width, height)
        self.selection_list = pygame_gui.elements.UISelectionList(
            relative_rect=self.rect,
            item_list=cast(list[str | tuple[str, str]], item_list),
            allow_multi_select=False,
            manager=manager,
            container=control_container
        )
        self._selected_command = selected_command
        self._current_selection = None

    def get_single_selection(self, include_object_id: bool = False) -> str | tuple[str, str] | None:
        return self.selection_list.get_single_selection(include_object_id)
        

    def process_event(self, event: pygame.Event):
        if (event.type == pygame_gui.UI_SELECTION_LIST_NEW_SELECTION
            and event.ui_element == self.selection_list):
            selected = self.get_single_selection(True)
            if selected == None or type(selected) == tuple:
                self._current_selection = selected
                self._selected_command(selected)
            return
        elif (event.type == pygame_gui.UI_SELECTION_LIST_DROPPED_SELECTION
              and event.ui_element == self.selection_list):
            if self._selected_command == None or self.get_single_selection() == None:
                self._selected_command(None)
            return

    def disable(self):
        self.selection_list.disable()

    def enable(self):
        self.selection_list.enable()

    def hide(self):
        self.selection_list.hide()

    def show(self):
        self.selection_list.show()

    def kill(self):
        self.selection_list.kill()
