import pygame, pygame_gui
from src.control.core import Control
from src.preview import Preview

class MenuDesigner:
    manager: pygame_gui.UIManager
    preview: Preview
    control: Control

    def __init__(self, manager: pygame_gui.UIManager):
        self.manager = manager
        self.preview = Preview(self.manager)
        self.control = Control(self.manager, self.preview)

    def update(self, time_delta: float):
        self.preview.update(time_delta)
        self.control.update(time_delta)

    def process_event(self, event: pygame.Event):
        self.preview.process_event(event)
        self.control.process_event(event)

    def draw(self, surface: pygame.Surface):
        self.preview.draw(surface)
