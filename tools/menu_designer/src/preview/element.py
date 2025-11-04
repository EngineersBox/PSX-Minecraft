import pygame, pygame_gui
from src.preview.const import PREVIEW_SCALE_X, PREVIEW_SCALE_Y

PREVIEW_OUTLINE_COLOUR = pygame.color.Color("#34c0eb")

class PreviewElement:
    rect: pygame.Rect
    surface: pygame.Surface
    image: pygame_gui.elements.UIImage

    _width: int
    _height: int
    draw_outline: bool
    outline_offset_x: int
    outline_offset_y: int

    def __init__(
        self,
        x: int,
        y: int,
        width: int,
        height: int,
        surface: pygame.Surface,
        manager: pygame_gui.UIManager,
        preview_container: pygame_gui.elements.UIPanel,
        outline_offset_x: int = 5,
        outline_offset_y: int = 5
    ):
        self.rect = pygame.Rect(
            x,
            y,
            width,
            height
        )
        self.surface = surface
        self.image = pygame_gui.elements.UIImage(
            relative_rect=self.rect,
            image_surface=self.surface,
            manager=manager,
            container=preview_container
        )
        self._width = width
        self._height = height
        self.draw_outline = False
        self.outline_offset_x = outline_offset_x
        self.outline_offset_y = outline_offset_y

    def get_render_index(self) -> int:
        return self.image.starting_height

    def set_render_index(self, height: int):
        self.image.starting_height = height
        self.image.change_layer(height)

    def update(self, time_delta: float):
        self.image.update(time_delta)

    def process_event(self, event: pygame.Event):
        self.image.process_event(event)

    def move(self, move: tuple[int, int], preview_rect: pygame.Rect | pygame.FRect):
        rel_rect = self.image.get_relative_rect()
        if (preview_rect.contains(
            rel_rect.x + move[0],
            rel_rect.y + move[1],
            rel_rect.width,
            rel_rect.height
        )):
            self.image.set_relative_position((
                rel_rect.x + move[0],
                rel_rect.y + move[1]
            ))

    def get_width(self) -> int:
        return int(self.image.get_abs_rect().width) // PREVIEW_SCALE_X

    def get_height(self) -> int:
        return int(self.image.get_abs_rect().height) // PREVIEW_SCALE_Y

    def get_pos(self) -> tuple[int, int]:
        pos = self.image.get_relative_rect()
        return (
            int(pos.x) // PREVIEW_SCALE_X,
            int(pos.y) // PREVIEW_SCALE_Y
        )

    def set_width(self, width: int):
        self._width = width * PREVIEW_SCALE_X
        self.image.set_dimensions((
            width * PREVIEW_SCALE_X,
            self.image.get_abs_rect().height
        ))

    def set_height(self, height: int):
        self._height = height * PREVIEW_SCALE_Y
        self.image.set_dimensions((
            self.image.get_abs_rect().width,
            height * PREVIEW_SCALE_Y
        ))

    def draw(self, surface: pygame.Surface):
        if not self.draw_outline:
            return
        pos = self.image.get_abs_rect()
        points = [
            (
                pos.x - self.outline_offset_x,
                pos.y - self.outline_offset_y
            ),
            (
                pos.x + pos.width + self.outline_offset_x,
                pos.y - self.outline_offset_y
            ),
            (
                pos.x + pos.width + self.outline_offset_x,
                pos.y + pos.height + self.outline_offset_y
            ),
            (
                pos.x - self.outline_offset_x,
                pos.y + pos.height + self.outline_offset_y
            )
        ]
        pygame.draw.lines(
            surface,
            PREVIEW_OUTLINE_COLOUR,
            True,
            points,
            2
        )

    def kill(self):
        self.image.kill()

    def point_intersects(self, pos: tuple[int, int]) -> bool:
        return self.image.hover_point(pos[0], pos[1])
