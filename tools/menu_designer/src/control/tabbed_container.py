from typing import Callable, Optional, TypeVar
import pygame, pygame_gui
from src.control.base import ControlBase
from src.control.button import ControlButton

T = TypeVar("T", bound="ControlBase")
type ControlTabbedContainerAddTabCommand[T] = Callable[[pygame.Rect, pygame_gui.core.IContainerLikeInterface], tuple[str, T]]

class ControlTabbedContainer(ControlBase):
    container: pygame_gui.core.UIContainer
    tab_buttons_container: pygame_gui.core.UIContainer
    tabs: dict[str, tuple[ControlButton, ControlBase]]

    _manager: pygame_gui.UIManager

    def __init__(
        self,
        x: int,
        y: int,
        width: int,
        height: int,
        manager: pygame_gui.UIManager,
        parent_container: Optional[pygame_gui.core.IContainerLikeInterface] = None
    ):
        super().__init__(
            x,
            y,
            width,
            height
        )
        self.container = pygame_gui.core.UIContainer(
            relative_rect=self.rect,
            manager=manager,
            container=parent_container
        )
        self.tab_buttons_container = pygame_gui.core.UIContainer(
            relative_rect=pygame.Rect(
                0,
                0,
                width,
                30
            ),
            manager=manager,
            container=self.container
        )
        self._manager = manager
        self.tabs = {}

    def process_event(self, event: pygame.Event):
        super().process_event(event)
        self.container.process_event(event)
        for (button, element) in self.tabs.values():
            button.process_event(event)
            element.process_event(event)

    def set_active(self, tab_name: str):
        self._tab_pressed(tab_name)

    def add_tab(self, add_tab_command: ControlTabbedContainerAddTabCommand[T]) -> T:
        tab_name, element = add_tab_command(
            pygame.Rect(
                0,
                self.tab_buttons_container.get_relative_rect().height,
                self.rect.width,
                self.rect.height - self.tab_buttons_container.get_relative_rect().height
            ),
            self.container
        )
        button = ControlButton(
            0, 0, 0, 30,
            tab_name,
            self._manager,
            self.tab_buttons_container,
            lambda: self._tab_pressed(tab_name)
        )
        self.tabs[tab_name] = (button, element)
        self._adjust_tab_dimensions()
        return element

    def get_tab_element(self, tab_name: str) -> ControlBase:
        return self.tabs[tab_name][1]

    def _tab_pressed(self, tab_name: str):
        for (button, element) in self.tabs.values():
            button.enable()
            element.disable()
            element.hide()
        button, element = self.tabs[tab_name]
        button.disable()
        element.show()
        element.enable()

    def _adjust_tab_dimensions(self):
        count = len(self.tabs)
        tab_width = self.rect.width // count
        x_offset = 0
        for (button, _) in self.tabs.values():
            button.button.set_relative_position((x_offset, 0))
            button.button.set_dimensions((tab_width, button.rect.height))
            x_offset += tab_width

    def enable(self):
        self.container.enable()
        self.tab_buttons_container.enable()
        for (button, element) in self.tabs.values():
            button.enable()
            element.enable()

    def disable(self):
        self.container.disable()
        self.tab_buttons_container.disable()
        for (button, element) in self.tabs.values():
            button.disable()
            element.disable()

    def show(self):
        self.container.show()
        self.tab_buttons_container.show()
        for (button, element) in self.tabs.values():
            button.show()
            element.show()

    def hide(self):
        self.container.hide()
        self.tab_buttons_container.hide()
        for (button, element) in self.tabs.values():
            button.hide()
            element.hide()

    def kill(self):
        self.container.kill()
        self.tab_buttons_container.kill()
        for (button, element) in self.tabs.values():
            button.kill()
            element.kill()

