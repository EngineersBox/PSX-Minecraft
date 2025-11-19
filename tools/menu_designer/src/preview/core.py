from typing import Optional
import pygame, pygame_gui, uuid
from src.preview.background import PreviewBackground
from src.preview.element import PreviewElement
from src.preview.const import PREVIEW_SCALE_X, PREVIEW_SCALE_Y, PREVIEW_SIZE_X, PREVIEW_SIZE_Y
from src.preview.button import PreviewButton

PREVIEW_BG_COLOUR = pygame.color.Color("#211F1E")
PREVIEW_BORDER_COLOUR = pygame.color.Color("#EFEFEF")
PREVIEW_GRID_COLOUR = pygame.color.Color("#7F7F7F")

class Preview:
    manager: pygame_gui.UIManager
    button_normal_image: pygame.Surface
    button_disabled_image: pygame.Surface
    button_active_image: pygame.Surface
    game_elements: dict[str, PreviewElement]
    rect: pygame.Rect
    surface: pygame.Surface
    bg_image: Optional[pygame.Surface]
    container: pygame_gui.elements.UIPanel

    held_element: PreviewElement | None
    mouse_pos: tuple[int, int]

    hidden: bool
    disabled: bool
    grid_hidden: bool

    _render_index: int

    def __init__(self, manager: pygame_gui.UIManager):
        self.manager = manager
        self.button_normal_image = pygame.image.load("assets/button.png")
        self.button_disabled_image = pygame.image.load("assets/disabled_button.png")
        self.button_active_image = pygame.image.load("assets/active_button.png")
        self.game_elements = {}
        self.rect = pygame.Rect(
            0,
            0,
            PREVIEW_SIZE_X * PREVIEW_SCALE_X + 6,
            PREVIEW_SIZE_Y * PREVIEW_SCALE_Y + 6
        )
        self.surface = pygame.Surface((
            PREVIEW_SIZE_X * PREVIEW_SCALE_X,
            PREVIEW_SIZE_Y * PREVIEW_SCALE_Y
        ))
        self.bg_image = None
        self.container = pygame_gui.elements.UIPanel(
            relative_rect=self.rect,
            manager=manager
        )
        self.held_element = None
        self.mouse_pos = (0, 0)
        self.hidden = False
        self.disabled = False
        self.grid_hidden = True
        self.hide_grid()
        self._render_index = 1

    def update(self, time_delta: float):
        for button in self.game_elements.values():
            button.update(time_delta)

    def _get_intersected_button(self, pos: tuple[int, int]) -> PreviewElement | None:
        ordered_elements = reversed(list(filter(
            lambda element: element.image.get_starting_height(),
            list(self.game_elements.values())
        )))
        for element in ordered_elements:
            if element.point_intersects(pos):
                return element
        return None

    def process_event(self, event: pygame.Event):
        if self.hidden or self.disabled:
            return
        if (event.type == pygame.MOUSEBUTTONDOWN):
            self.held_element = self._get_intersected_button(pygame.mouse.get_pos())
            if self.held_element is not None:
                self.held_element._draw_outline = True
            self.mouse_pos = pygame.mouse.get_pos()
        elif (event.type == pygame.MOUSEBUTTONUP):
            if self.held_element is not None:
                self.held_element._draw_outline = False
            self.held_element = None
        elif (event.type == pygame.MOUSEMOTION
            and self.held_element is not None):
            next_mouse_pos = pygame.mouse.get_pos()
            move_x, move_y = 0, 0
            if (abs(next_mouse_pos[0] - self.mouse_pos[0]) >= PREVIEW_SCALE_X):
                move_x = ((next_mouse_pos[0] - self.mouse_pos[0]) // PREVIEW_SCALE_X) * PREVIEW_SCALE_X
                self.mouse_pos = (next_mouse_pos[0], self.mouse_pos[1])
            if (abs(next_mouse_pos[1] - self.mouse_pos[1]) >= PREVIEW_SCALE_Y):
                move_y = ((next_mouse_pos[1] - self.mouse_pos[1]) // PREVIEW_SCALE_Y) * PREVIEW_SCALE_Y
                self.mouse_pos = (self.mouse_pos[0], next_mouse_pos[1])
            self.held_element.move((move_x, move_y), pygame.Rect(
                0,
                0,
                PREVIEW_SIZE_X * PREVIEW_SCALE_X,
                PREVIEW_SIZE_Y * PREVIEW_SCALE_Y
            ))
        for button in self.game_elements.values():
            button.process_event(event)

    def add_button(self, text: str) -> str:
        button = PreviewButton(
            0,
            0,
            200,
            text,
            self.button_normal_image,
            self.button_disabled_image,
            self.button_active_image,
            self.manager,
            self.container
        )
        return self.add_element(button)

    def add_background(self, name: str, tim_name: str, bundle_name: str, image: pygame.Surface) -> str:
        background = PreviewBackground(
            name,
            0,
            0,
            100,
            100,
            tim_name,
            bundle_name,
            image,
            self.manager,
            self.container
        )
        return self.add_element(background)

    def add_element(self, element: PreviewElement) -> str:
        element_id = str(uuid.uuid4())
        self.game_elements[element_id] = element
        element.set_render_index(self._render_index)
        self._render_index += 1
        return element_id

    def remove_element(self, button_id: str):
        button = self.game_elements.pop(button_id)
        button.kill()
        
    def get_element(self, element_id: str) -> Optional[PreviewElement]:
        return self.game_elements[element_id]

    def get_all_elements(self) -> list[PreviewElement]:
        return list(self.game_elements.values())

    def set_held_element(self, element_id: str):
        self.held_element = self.game_elements[element_id]

    def draw(self, surface: pygame.Surface):
        if (not self.hidden and self.held_element is not None):
            self.held_element.draw(surface)

    def hide(self):
        self.hidden = True
        self.container.hide()

    def show(self):
        self.hidden = False
        self.container.show()

    def disable(self):
        self.disabled = True
        self.container.disable()

    def enable(self):
        self.disabled = False
        self.container.enable()

    def set_background_image(self, image: pygame.Surface):
        self.bg_image = pygame.transform.smoothscale(
            image,
            (PREVIEW_SIZE_Y * PREVIEW_SCALE_X, PREVIEW_SIZE_Y * PREVIEW_SCALE_Y)
        )
        if (self.grid_hidden):
            self.hide_grid()
        else:
            self.show_grid()

    def _draw_border(self):
        pygame.draw.lines(
            self.surface,
            PREVIEW_BORDER_COLOUR,
            True,
            [
                (0, 0),
                (self.rect.width + 6, 0),
                (self.rect.width + 6, self.rect.height + 6),
                (0, self.rect.height + 6)
            ]
        )

    def hide_grid(self):
        self.surface.fill(PREVIEW_BG_COLOUR)
        if self.bg_image is not None:
            self.surface.blit(
                self.bg_image,
                pygame.Rect(
                    0,
                    0,
                    PREVIEW_SIZE_Y * PREVIEW_SCALE_X,
                    PREVIEW_SIZE_Y * PREVIEW_SCALE_Y
                )
            )
        self.container.set_image(self.surface)
        self.grid_hidden = True

    def show_grid(self):
        self.surface.fill(PREVIEW_BG_COLOUR)
        if self.bg_image is not None:
            self.surface.blit(
                self.bg_image,
                pygame.Rect(
                    0,
                    0,
                    PREVIEW_SIZE_Y * PREVIEW_SCALE_X,
                    PREVIEW_SIZE_Y * PREVIEW_SCALE_Y
                )
            )
        for x in range(PREVIEW_SCALE_X, PREVIEW_SIZE_X * PREVIEW_SCALE_X, PREVIEW_SCALE_X):
            pygame.draw.line(
                self.surface,
                pygame.color.Color(PREVIEW_GRID_COLOUR),
                (x, PREVIEW_SCALE_Y),
                (x, PREVIEW_SIZE_Y * PREVIEW_SCALE_Y),
                1
            )
        for y in range(PREVIEW_SCALE_Y, PREVIEW_SIZE_Y * PREVIEW_SCALE_Y, PREVIEW_SCALE_Y):
            pygame.draw.line(
                self.surface,
                pygame.color.Color(PREVIEW_GRID_COLOUR),
                (PREVIEW_SCALE_X, y),
                (PREVIEW_SIZE_X * PREVIEW_SCALE_X, y),
                1
            )
        self.container.set_image(self.surface)
        self.grid_hidden = False
