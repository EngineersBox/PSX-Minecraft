import pygame, pygame_gui, math, cv2
import numpy as np
from src.preview.const import PREVIEW_SCALE_X, PREVIEW_SCALE_Y, PREVIEW_SIZE_Y
from src.preview.element import PreviewElement

def normalised_dual_degrees(value: float) -> float:
    return ((value * 2.0) - 360.0) / 360.0

def normalised_dual_unit(value: float) -> float:
    return ((value * 2.0) - 100.0) / 100.0

class PreviewBackground(PreviewElement):
    source_image: pygame.Surface

    _name: str
    _pos_u: int
    _pos_v: int
    _tile_x: int
    _tile_y: int
    _tint: pygame.Color
    _direct_blit: bool

    def __init__(
        self,
        name: str,
        x: int,
        y: int,
        width: int,
        height: int,
        source_image: pygame.Surface,
        manager: pygame_gui.UIManager,
        preview_container: pygame_gui.elements.UIPanel,
    ):
        assert width > 0 and width <= 256
        assert height > 0 and height < PREVIEW_SIZE_Y
        self.source_image = source_image
        render_surface = pygame.Surface((
            width * PREVIEW_SCALE_X,
            height * PREVIEW_SCALE_Y
        ))
        self._name = name
        self._tint = pygame.Color("#808080")
        super().__init__(
            x,
            y,
            width,
            height,
            render_surface,
            manager,
            preview_container
        )
        self._pos_u = 0
        self._pos_v = 0
        self._tile_x = 256
        self._tile_y = 256
        self._direct_blit = True
        self._tile_image()

    def _tile_image(self):
        if self._direct_blit:
            self.surface = pygame.Surface((
                self._width,
                self._height
            ))
            source = self._surface_to_cv2_mat(pygame.transform.scale(
                self.source_image,
                (self._width, self._height)
            ))
            self._blend_blit(
                self.surface,
                source,
                (0, 0)
            )
            self.image.set_image(self.surface)
            return
        tile_x = self._tile_x * PREVIEW_SCALE_X
        tile_y = self._tile_y * PREVIEW_SCALE_Y
        tile = self.source_image.subsurface(pygame.Rect(
            self._pos_u,
            self._pos_v,
            min(self._tile_x, self.source_image.width),
            min(self._tile_y, self.source_image.height)
        ))
        tile = self._surface_to_cv2_mat(pygame.transform.scale(
            tile,
            (tile_x, tile_y)
        ))
        self.surface = pygame.Surface((
            self._width,
            self._height
        ))
        u_count = max(1, math.ceil(self._width / tile_x))
        v_count = max(1, math.ceil(self._height / tile_y))
        for i in range(u_count):
            for j in range(v_count):
                self._blend_blit(
                    self.surface,
                    tile,
                    (tile_x * i, tile_y * j)
                )
        self.image.set_image(self.surface)

    def _surface_to_cv2_mat(self, image: pygame.Surface) -> cv2.typing.MatLike:
        view = pygame.surfarray.array3d(image)
        # Convert from (width, height, channel) to (height, width, channel)
        view = view.transpose([1, 0, 2])
        return cv2.cvtColor(view, cv2.COLOR_RGB2RGBA)
    
    def _blend_blit(
        self,
        dest: pygame.Surface,
        src: cv2.typing.MatLike,
        pos: tuple[int, int]
    ):
        # https://psx-spx.consoledev.net/graphicsprocessingunitgpu/#modulation-also-known-as-texture-blending
        img = cv2.multiply(
            src,
            np.array([
                self._tint.r / 128,
                self._tint.g / 128,
                self._tint.b / 128,
                1
            ])
        )
        img = pygame.image.frombuffer(img.tostring(), img.shape[1::-1], "RGBA")
        dest.blit(img, pos)

    def get_name(self) -> str:
        return self._name

    def set_direct_blit(self, state: bool):
        self._direct_blit = state
        self._tile_image()

    def get_direct_blit(self) -> bool:
        return self._direct_blit

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

    def set_width(self, width: int):
        super().set_width(width)
        self._tile_image()

    def set_height(self, height: int):
        super().set_height(height)
        self._tile_image()

    def set_tint(self, tint: pygame.Color):
        self._tint = tint
        self._tile_image()

    def get_tint(self) -> pygame.Color:
        return self._tint
