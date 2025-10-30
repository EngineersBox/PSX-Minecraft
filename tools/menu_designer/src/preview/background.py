import pygame, pygame_gui
from src.preview.element import PreviewElement
from src.preview.const import PREVIEW_SCALE_X, PREVIEW_SCALE_Y

class PreviewBackground(PreviewElement):
    surface: pygame.Surface
    image: pygame_gui.elements.UIImage

    def __init__(
        self,
        x: int,
        y: int,
        width: int,
        height: int,
        surface: pygame.Surface,
        manager: pygame_gui.UIManager,
        preview_container: pygame_gui.elements.UIPanel,
    ):
        super().__init__(
            x,
            y,
            width,
            height,
            surface,
            manager,
            preview_container
        )
