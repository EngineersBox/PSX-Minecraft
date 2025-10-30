import pygame
import pygame_gui
from src.control.base import ControlBase
from typing import Optional, TypeVar

T = TypeVar("T", bound="ControlBase")

class ControlPanel(ControlBase):
    surface: pygame.Surface
    panel: pygame_gui.elements.UIPanel
    elements: list[ControlBase]

    _update_elements: dict[str, ControlBase]
    _process_event_elements: dict[str, ControlBase]

    def __init__(
        self,
        x: int,
        y: int,
        width: int,
        height: int,
        manager: pygame_gui.UIManager,
        container: Optional[pygame_gui.core.IContainerLikeInterface] = None,
        background_colour: Optional[pygame.Color] = None
    ):
        super().__init__(
            x,
            y,
            width,
            height
        )
        self.panel = pygame_gui.elements.UIPanel(
            relative_rect=self.rect,
            manager=manager,
            container=container
        )
        if background_colour is not None:
            self.surface = pygame.Surface((width, height))
            self.surface.fill(background_colour)
            self.panel.set_background_images([self.surface])
        self.elements = []
        self._update_elements = {}
        self._process_event_elements = {}
            
    def add_element(
        self,
        element: T,
        update: bool = False,
        process_event: bool = False
    ) -> T:
        self.elements.append(element)
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

    def disable(self):
        self.panel.disable()
        for element in self.elements:
            element.disable()

    def enable(self):
        self.panel.enable()
        for element in self.elements:
            element.enable()

    def show(self):
        self.panel.show()
        for element in self.elements:
            element.show()

    def hide(self):
        self.panel.hide()
        for element in self.elements:
            element.hide()

    def kill(self):
        self.panel.kill()
