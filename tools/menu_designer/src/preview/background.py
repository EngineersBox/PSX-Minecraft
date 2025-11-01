import pygame, pygame_gui
from src.preview.element import PreviewElement

class PreviewBackground(PreviewElement):
    surface: pygame.Surface

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
