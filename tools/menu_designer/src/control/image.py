import pygame, pygame_gui
from src.control.base import ControlBase

class ControlImage(ControlBase):
    image: pygame_gui.elements.UIImage

    _surface: pygame.Surface

    def __init__(
        self,
        x: int,
        y: int,
        width: int,
        height: int,
        manager: pygame_gui.UIManager,
        container: pygame_gui.core.IContainerLikeInterface
    ):
        super().__init__(
            x,
            y,
            width,
            height
        )
        self._surface = pygame.Surface((width, height))
        self._surface.fill(pygame.Color("#000000"))
        self.image = pygame_gui.elements.UIImage(
            relative_rect=self.rect,
            image_surface=self._surface,
            manager=manager,
            container=container,
            scale_func=pygame.transform.scale
        )

    def set_image(self, path: str):
        self._surface = pygame.image.load(path)
        self.image.set_image(self._surface)

    def enable(self):
        self.image.enable()

    def disable(self):
        self.image.disable()

    def hide(self):
        self.image.hide()

    def show(self):
        self.image.show()

    def kill(self):
        self.image.kill()
