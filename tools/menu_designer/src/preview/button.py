import pygame, pygame_gui
from src.preview.element import PreviewElement
from src.preview.const import PREVIEW_SCALE_X, PREVIEW_SCALE_Y

class PreviewButton(PreviewElement):
    label: pygame_gui.elements.UILabel
    disabled: bool

    enabled_image: pygame.Surface
    disabled_image: pygame.Surface

    def __init__(
        self,
        x: int,
        y: int,
        width: int,
        text: str,
        enabled_image: pygame.Surface,
        disabled_image: pygame.Surface,
        manager: pygame_gui.UIManager,
        preview_container: pygame_gui.elements.UIPanel,
    ):
        self.enabled_image = enabled_image
        self.disabled_image = disabled_image
        super().__init__(
            x,
            y,
            width * PREVIEW_SCALE_X,
            20 * PREVIEW_SCALE_Y,
            pygame.transform.scale(
                self.enabled_image,
                (
                    width * PREVIEW_SCALE_X,
                    20 * PREVIEW_SCALE_Y
                )
            ),
            manager,
            preview_container
        )
        self.label = pygame_gui.elements.UILabel(
            relative_rect=self.rect,
            text=text,
            manager=manager,
            container=preview_container
        )
        self.label.starting_height = self.image.starting_height
        self.disabled = False

    def set_render_index(self, height: int):
        super().set_render_index(height)
        self.label.starting_height = height
        self.label.change_layer(height)

    def update(self, time_delta: float):
        super().update(time_delta)
        self.label.update(time_delta)

    def process_event(self, event: pygame.Event):
        super().process_event(event)
        self.label.process_event(event)

    def move(self, move: tuple[int, int], preview_rect: pygame.Rect | pygame.FRect):
        super().move(move, preview_rect)
        rel_rect = self.label.get_relative_rect()
        if (preview_rect.contains(
            rel_rect.x + move[0],
            rel_rect.y + move[1],
            rel_rect.width,
            rel_rect.height
        )):
            self.label.set_relative_position((
                rel_rect.x + move[0],
                rel_rect.y + move[1]
            ))

    def get_width(self) -> int:
        return int(self.image.get_abs_rect().width) // PREVIEW_SCALE_X

    def get_pos(self) -> tuple[int, int]:
        pos = self.image.get_relative_rect()
        return (
            int(pos.x) // PREVIEW_SCALE_X,
            int(pos.y) // PREVIEW_SCALE_Y
        )

    def set_width(self, width: int):
        super().set_width(width)
        self.label.set_dimensions((
            self._width,
            self.label.get_abs_rect().height
        ))
        self._update_image()

    def set_height(self, height: int):
        super().set_height(height)
        self.label.set_dimensions((
            self.label.get_abs_rect().width,
            self._height
        ))
        self._update_image()

    def kill(self):
        super().kill()
        self.label.kill()

    def set_disabled(self, disabled: bool):
        self.disabled = disabled
        self._update_image()

    def _update_image(self):
        self.image.set_image(pygame.transform.scale(
            self.disabled_image if self.disabled else self.enabled_image,
            (
                self._width,
                self._height
            )
        ))
