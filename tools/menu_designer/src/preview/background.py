import pygame, pygame_gui, math
from src.preview.const import PREVIEW_SIZE_Y
from src.preview.element import PreviewElement

class PreviewBackground(PreviewElement):
    source_image: pygame.Surface

    _pos_u: int
    _pos_v: int
    _tile_x: int
    _tile_y: int

    def __init__(
        self,
        x: int,
        y: int,
        width: int,
        height: int,
        source_image: pygame.Surface,
        manager: pygame_gui.UIManager,
        preview_container: pygame_gui.elements.UIPanel,
    ):
        assert width > 0 and width <= 246
        assert height > 0 and height < PREVIEW_SIZE_Y
        self.source_image = source_image
        super().__init__(
            x,
            y,
            width,
            height,
            pygame.transform.smoothscale(
                self.source_image,
                (width, height)
            ),
            manager,
            preview_container
        )
        self._pos_u = 0
        self._pos_v = 0
        self._tile_x = 256
        self._tile_y = 256

    def _tile_image(self):
        tile = pygame.transform.smoothscale(
            self.source_image,
            (
                max(self.source_image.width, self._tile_x),
                max(self.source_image.height, self._tile_y)
            )
        )
        self.surface.fill(pygame.color.Color("#000000"))
        u_count = max(1, math.ceil(self.surface.width / self._tile_x))
        v_count = max(1, math.ceil(self.surface.height / self._tile_y))
        for i in range(u_count):
            for j in range(v_count):
                self.surface.blit(
                    tile,
                    (self._tile_x * i, self._tile_y * j),
                    pygame.Rect(
                        self._pos_u,
                        self._pos_v,
                        self._tile_x,
                        self._tile_y
                    )
                )
        self.image.set_image(self.surface)

    def set_pos_u(self, u: int):
        self._pos_u = u
        self._tile_image()

    def get_pos_u(self) -> int:
        return self._pos_u

    def set_pos_v(self, v: int):
        self._pos_v = v
        self._tile_image()

    def get_pos_v(self) -> int:
        return self._pos_v

    def set_tile_x(self, tile_x: int):
        self._tile_x = tile_x
        self._tile_image()

    def get_tile_x(self) -> int:
        return self._tile_x

    def set_tile_y(self, tile_y: int):
        self._tile_y = tile_y
        self._tile_image()

    def get_tile_y(self) -> int:
        return self._tile_y
