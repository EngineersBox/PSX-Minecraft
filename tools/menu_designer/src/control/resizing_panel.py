import pygame_gui
from typing import Callable, TypeVar
from src.control.base import ControlBase
from src.control.panel import ControlPanel

T = TypeVar("T", bound="ControlBase")

type ElementCreateFn[T] = Callable[[int, int, pygame_gui.core.IContainerLikeInterface], T]

class ControlResizingPanel(ControlPanel):
    x_offset: int
    y_offset: int

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
        self.y_offset = 0

    def add_element_offset(
        self,
        element_create_fn: ElementCreateFn[T],
        update: bool = False,
        process_event: bool = False,
        y_spacing: int = 0,
        update_y_offset: bool = True
    ) -> T:
        element = element_create_fn(
            self.rect.width - 6,
            self.y_offset,
            self.panel
        )
        if update_y_offset:
            self.y_offset += element.rect.height + y_spacing
        self.panel.set_dimensions((
            self.panel.get_abs_rect().width,
            self.y_offset + 3
        ))
        return self.add_element(element, update, process_event)
