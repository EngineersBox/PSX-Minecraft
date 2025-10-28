import pygame, pygame_gui
from src.control.base import ControlBase

class ControlHorizontalSlider(ControlBase):
    slider: pygame_gui.elements.UIHorizontalSlider

    def __init__(
        self,
        x: int,
        y: int,
        width: int,
        height: int,
        value_range: tuple[int, int],
        increment: int,
        manager: pygame_gui.UIManager,
        container: pygame_gui.core.IContainerLikeInterface
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

    def enable(self):
        self.slider.enable()

    def disable(self):
        self.slider.disable()

    def set_value(self, value: int):
        if (value < self.slider.value_range[0]
            or value > self.slider.value_range[1]):
            value = int((self.slider.value_range[0] + self.slider.value_range[1]) / 2)
        self.slider.set_current_value(value)

    def get_value(self) -> int:
        return int(self.slider.current_value)
