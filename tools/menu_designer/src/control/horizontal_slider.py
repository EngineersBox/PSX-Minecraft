from typing import Callable
import pygame, pygame_gui
from src.control.base import ControlBase

type ControlHorizontalSliderChangedCommand = Callable[[int], None]

class ControlHorizontalSlider(ControlBase):
    slider: pygame_gui.elements.UIHorizontalSlider

    _changed_command: ControlHorizontalSliderChangedCommand

    def __init__(
        self,
        x: int,
        y: int,
        width: int,
        height: int,
        value_range: tuple[int, int],
        increment: int,
        manager: pygame_gui.UIManager,
        container: pygame_gui.core.IContainerLikeInterface,
        changed_command: ControlHorizontalSliderChangedCommand
    ):
        super().__init__(
            x,
            y,
            width,
            height
        )
        self.slider = pygame_gui.elements.UIHorizontalSlider(
            relative_rect=self.rect,
            start_value=0,
            value_range=value_range,
            click_increment=increment,
            manager=manager,
            container=container
        )
        self._changed_command = changed_command

    def process_event(self, event: pygame.Event):
        if (event.type == pygame_gui.UI_HORIZONTAL_SLIDER_MOVED
            and event.ui_element == self.slider):
            self._changed_command(int(self.slider.current_value))

    def enable(self):
        self.slider.enable()

    def disable(self):
        self.slider.disable()

    def hide(self):
        self.slider.hide()

    def show(self):
        self.slider.show()

    def set_value(self, value: int):
        if (value < self.slider.value_range[0]
            or value > self.slider.value_range[1]):
            value = int((self.slider.value_range[0] + self.slider.value_range[1]) / 2)
        self.slider.set_current_value(value)

    def get_value(self) -> int:
        return int(self.slider.current_value)
