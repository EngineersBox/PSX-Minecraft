import pygame, uuid

import pygame_gui

class ControlBase():
    element_id: str
    rect: pygame.Rect

    def __init__(
        self,
        x: int,
        y: int,
        width: int,
        height: int
    ):
        self.element_id = str(uuid.uuid4())
        self.rect = pygame.Rect(
            x,
            y,
            width,
            height
        )

    def update(self, time_delta: float):
        pass

    def process_event(self, event: pygame.Event):
        pass
