from typing import Optional
import pygame, pygame_gui, uuid

PREVIEW_SCALE_X = 3
PREVIEW_SCALE_Y = 3

class PreviewButton:
    rect: pygame.Rect
    image: pygame_gui.elements.UIImage
    label: pygame_gui.elements.UILabel
    draw_outline: bool
    outline_offset_x: int
    outline_offset_y: int

    def __init__(
        self,
        x: int,
        y: int,
        width: int,
        text: str,
        image: pygame.Surface,
        manager: pygame_gui.UIManager,
        preview_container: pygame_gui.elements.UIPanel,
        outline_offset_x: int = 5,
        outline_offset_y: int = 5
    ):
        self.rect = pygame.Rect(
            x,
            y,
            width * PREVIEW_SCALE_X,
            20 * PREVIEW_SCALE_Y
        )
        self.image = pygame_gui.elements.UIImage(
            relative_rect=self.rect,
            image_surface=image,
            manager=manager,
            container=preview_container
        )
        self.label = pygame_gui.elements.UILabel(
            relative_rect=self.rect,
            text=text,
            manager=manager,
            container=preview_container
        )
        self.draw_outline = False
        self.outline_offset_x = outline_offset_x
        self.outline_offset_y = outline_offset_y

    def update(self, time_delta: float):
        self.image.update(time_delta)
        self.label.update(time_delta)

    def process_event(self, event: pygame.Event):
        self.image.process_event(event)
        self.label.process_event(event)

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
            rel_rect = self.label.get_relative_rect()
            self.label.set_relative_position((
                rel_rect.x + move[0],
                rel_rect.y + move[1]
            ))

    def get_width(self) -> int:
        return int(self.image.get_abs_rect().width)

    def set_width(self, width: int):
        width = ((width // PREVIEW_SCALE_X) * PREVIEW_SCALE_X)
        self.image.set_dimensions((
            width,
            self.image.get_abs_rect().height
        ))
        self.label.set_dimensions((
            width,
            self.label.get_abs_rect().height
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
            pygame.Color(0, 30, 200),
            True,
            points,
            2
        )

    def kill(self):
        self.image.kill()
        self.label.kill()

class Preview:
    manager: pygame_gui.UIManager
    button_image: pygame.Surface
    game_buttons: dict[str, PreviewButton]
    rect: pygame.Rect
    surface: pygame.Surface
    container: pygame_gui.elements.UIPanel

    held_button: PreviewButton | None
    mouse_pos: tuple[int, int]

    def __init__(self, manager: pygame_gui.UIManager):
        self.manager = manager
        self.button_image = pygame.image.load("assets/button.png")
        self.game_buttons = {}
        self.rect = pygame.Rect(0, 0, 320 * PREVIEW_SCALE_X, 240 * PREVIEW_SCALE_Y)
        self.surface = pygame.Surface((self.rect.width, self.rect.height))
        self.surface.fill(pygame.Color("#AFAFAF"))
        self.container = pygame_gui.elements.UIPanel(relative_rect=self.rect)
        self.held_button = None
        self.mouse_pos = (0, 0)

    def update(self, time_delta: float):
        for button in self.game_buttons.values():
            button.update(time_delta)

    def _get_intersected_button(self, pos: tuple[int, int]) -> PreviewButton | None:
        ordered_buttons = reversed(list(filter(
            lambda button: button.image.get_starting_height(),
            list(self.game_buttons.values())
        )))
        for button in ordered_buttons:
            if (button.image.hover_point(pos[0], pos[1])):
                return button
        return None

    def process_event(self, event: pygame.Event):
        if (event.type == pygame.MOUSEBUTTONDOWN):
            self.held_button = self._get_intersected_button(pygame.mouse.get_pos())
            if self.held_button is not None:
                self.held_button.draw_outline = True
            self.mouse_pos = pygame.mouse.get_pos()
        elif (event.type == pygame.MOUSEBUTTONUP):
            if self.held_button is not None:
                self.held_button.draw_outline = False
            self.held_button = None
        elif (event.type == pygame.MOUSEMOTION
            and self.held_button is not None):
            next_mouse_pos = pygame.mouse.get_pos()
            move_x, move_y = 0, 0
            if (abs(next_mouse_pos[0] - self.mouse_pos[0]) >= PREVIEW_SCALE_X):
                move_x = ((next_mouse_pos[0] - self.mouse_pos[0]) // PREVIEW_SCALE_X) * PREVIEW_SCALE_X
                self.mouse_pos = (next_mouse_pos[0], self.mouse_pos[1])
            if (abs(next_mouse_pos[1] - self.mouse_pos[1]) >= PREVIEW_SCALE_Y):
                move_y = ((next_mouse_pos[1] - self.mouse_pos[1]) // PREVIEW_SCALE_Y) * PREVIEW_SCALE_Y
                self.mouse_pos = (self.mouse_pos[0], next_mouse_pos[1])
            self.held_button.move((move_x, move_y), self.container.get_relative_rect())
        for button in self.game_buttons.values():
            button.process_event(event)

    def add_button(self, text: str) -> str:
        button = PreviewButton(
            0,
            0,
            200,
            text,
            self.button_image,
            self.manager,
            self.container
        )
        button_id = str(uuid.uuid4())
        self.game_buttons[button_id] = button
        return button_id

    def remove_button(self, button_id: str):
        button = self.game_buttons.pop(button_id)
        button.kill()
        
    def get_button(self, button_id: str) -> Optional[PreviewButton]:
        return self.game_buttons[button_id]

    def draw(self, surface: pygame.Surface):
        if self.held_button is not None:
            self.held_button.draw(surface)
