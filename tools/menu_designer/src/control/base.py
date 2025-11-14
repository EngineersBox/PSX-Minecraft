import pygame, uuid
from abc import ABC, abstractmethod

class ControlBase(ABC):
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

    @abstractmethod
    def disable(self):
        pass

    @abstractmethod
    def enable(self):
        pass

    @abstractmethod
    def hide(self):
        pass

    @abstractmethod
    def show(self):
        pass

    @abstractmethod
    def kill(self):
        pass
