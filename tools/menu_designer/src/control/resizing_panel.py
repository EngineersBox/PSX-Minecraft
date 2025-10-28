import pygame, pygame_gui
from typing import Callable, TypeVar
from src.control.base import ControlBase
from src.control.panel import ControlPanel

T = TypeVar("T", bound="ControlBase")

type ElementCreateFn[T] = Callable[[int, int, pygame_gui.core.IContainerLikeInterface], T]

class ControlResizingPanel(ControlPanel):
    elements: list[ControlBase]
    y_offset: int

    _update_elements: dict[str, ControlBase]
    _process_event_elements: dict[str, ControlBase]

    def __init__(
        self,
        x: int,
        y: int,
        width: int,
        height: int,
        manager: pygame_gui.UIManager,
        container: pygame_gui.core.IContainerLikeInterface | None = None,
    ):
        super().__init__(
            x,
            y,
            width,
            height,
            manager,
            container
        )
        self.elements = []
        self._update_elements = {}
        self._process_event_elements = {}
        self.y_offset = 0

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

    def add_element_offset(
        self,
        element_create_fn: ElementCreateFn[T],
        update: bool = False,
        process_event: bool = False,
        y_spacing: int = 0
    ) -> T:
        element = element_create_fn(
            self.rect.width - 6,
            self.y_offset,
            self.panel
        )
        self.y_offset += element.rect.height + y_spacing
        self.panel.set_dimensions((
            self.panel.get_abs_rect().width,
            self.y_offset + 3
        ))
        return self.add_element(element, update, process_event)

    def update(self, time_delta: float):
        for element in self._update_elements.values():
            element.update(time_delta)

    def process_event(self, event: pygame.Event):
        for element in self._process_event_elements.values():
            element.process_event(event)
