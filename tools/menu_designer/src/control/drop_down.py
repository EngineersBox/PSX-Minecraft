import pygame, pygame_gui
from src.control.base import ControlBase
from typing import Callable, List, Tuple

type ControlDropDownSelectedCommand = Callable[[Tuple[str, str]], None]

class ControlDropDown(ControlBase):
    drop_down: pygame_gui.elements.UIDropDownMenu
    selected_command: ControlDropDownSelectedCommand

    def __init__(
        self,
        x: int,
        y: int,
        width: int,
        height: int,
        options: List[str | Tuple[str, str]],
        selected_command: ControlDropDownSelectedCommand,
        manager: pygame_gui.UIManager,
        container: pygame_gui.core.IContainerLikeInterface,
        expand_upward: bool = False
    ):
        super().__init__(
            x,
            y,
            width,
            height
        )
        self.drop_down = pygame_gui.elements.UIDropDownMenu(
            relative_rect=self.rect,
            options_list=options,
            starting_option=options[0],
            manager=manager,
            container=container
        )
        self.drop_down.expand_direction = "down" if expand_upward else "up"
        self.selected_command = selected_command

    def process_event(self, event: pygame.Event):
        if (event.type == pygame_gui.UI_DROP_DOWN_MENU_CHANGED
            and event.ui_element == self.drop_down):
            self.selected_command(self.drop_down.selected_option)

    def disable(self):
        self.drop_down.disable()

    def enable(self):
        self.drop_down.enable()

    def hide(self):
        self.drop_down.hide()

    def show(self):
        self.drop_down.show()

    def kill(self):
        self.drop_down.kill()
