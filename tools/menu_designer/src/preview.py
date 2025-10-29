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
            image_surface=enabled_image,
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
        self.disabled = False
        self.enabled_image = enabled_image
        self.disabled_image = disabled_image

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
        return int(self.image.get_abs_rect().width) // PREVIEW_SCALE_X

    def get_pos(self) -> tuple[int, int]:
        pos = self.image.get_relative_rect()
        return (
            int(pos.x) // PREVIEW_SCALE_X,
            int(pos.y) // PREVIEW_SCALE_Y
        )

    def set_width(self, width: int):
        width = width * PREVIEW_SCALE_X
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

    def point_intersects(self, pos: tuple[int, int]) -> bool:
        return self.image.hover_point(pos[0], pos[1])

    def set_disabled(self, disabled: bool):
        self.disabled = disabled
        if self.disabled:
            self.image.set_image(self.disabled_image)
        else:
            self.image.set_image(self.enabled_image)

class Preview:
    manager: pygame_gui.UIManager
    button_enabled_image: pygame.Surface
    button_disabled_image: pygame.Surface
    game_buttons: dict[str, PreviewButton]
    rect: pygame.Rect
    surface: pygame.Surface
    bg_image: Optional[pygame.Surface]
    container: pygame_gui.elements.UIPanel

    held_button: PreviewButton | None
    mouse_pos: tuple[int, int]

    hidden: bool
    grid_hidden: bool

    def __init__(self, manager: pygame_gui.UIManager):
        self.manager = manager
        self.button_enabled_image = pygame.image.load("assets/button.png")
        self.button_disabled_image = pygame.image.load("assets/disabled_button.png")
        self.game_buttons = {}
        self.rect = pygame.Rect(0, 0, 320 * PREVIEW_SCALE_X, 240 * PREVIEW_SCALE_Y)
        self.surface = pygame.Surface((self.rect.width, self.rect.height))
        self.bg_image = None
        self.container = pygame_gui.elements.UIPanel(
            relative_rect=self.rect,
            manager=manager
        )
        self.held_button = None
        self.mouse_pos = (0, 0)
        self.hidden = False
        self.grid_hidden = True
        self.hide_grid()

    def update(self, time_delta: float):
        for button in self.game_buttons.values():
            button.update(time_delta)

    def _get_intersected_button(self, pos: tuple[int, int]) -> PreviewButton | None:
        ordered_buttons = reversed(list(filter(
            lambda button: button.image.get_starting_height(),
            list(self.game_buttons.values())
        )))
        for button in ordered_buttons:
            if button.point_intersects(pos):
                return button
        return None

    def process_event(self, event: pygame.Event):
        if self.hidden:
            return
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
            self.button_enabled_image,
            self.button_disabled_image,
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

    def set_held_button(self, button_id: str):
        self.held_button = self.game_buttons[button_id]

    def draw(self, surface: pygame.Surface):
        if (not self.hidden and self.held_button is not None):
            self.held_button.draw(surface)

    def hide(self):
        self.hidden = True
        self.container.hide()

    def show(self):
        self.hidden = False
        self.container.show()

    def set_background_image(self, image: pygame.Surface):
        self.bg_image = image
        if (self.grid_hidden):
            self.hide_grid()
        else:
            self.show_grid()

    def _draw_border(self):
        pygame.draw.lines(
            self.surface,
            pygame.color.Color("#EFEFEF"),
            True,
            [
                (0, 0),
                (self.rect.width, 0),
                (self.rect.width, self.rect.height),
                (0, self.rect.height)
            ]
        )

    def hide_grid(self):
        self.surface.fill(pygame.color.Color("#211F1E"))
        if self.bg_image is not None:
            self.surface.blit(
                self.bg_image,
                pygame.Rect(
                    0,
                    0,
                    self.rect.width,
                    self.rect.height
                )
            )
        self.container.set_background_images([self.surface])
        self.grid_hidden = True

    def show_grid(self):
        self.surface.fill(pygame.color.Color("#211F1E"))
        if self.bg_image is not None:
            # TODO: Make this span the entire surface
            self.surface.blit(
                self.bg_image,
                pygame.Rect(
                    0,
                    0,
                    self.rect.width,
                    self.rect.height
                )
            )
        # FIXME: These don't render for some reason
        for x in range(PREVIEW_SCALE_X, 320 * PREVIEW_SCALE_X, PREVIEW_SCALE_X):
            pygame.draw.line(
                self.surface,
                pygame.color.Color("#7F7F7F"),
                (x, 0),
                (x, 240 * PREVIEW_SCALE_Y),
                1
            )
        for y in range(PREVIEW_SCALE_Y, 240 * PREVIEW_SCALE_Y, PREVIEW_SCALE_Y):
            pygame.draw.line(
                self.surface,
                pygame.color.Color("#7F7F7F"),
                (0, y),
                (320 * PREVIEW_SCALE_X, y),
                1
            )
        self.container.set_background_images([self.surface])
        self.grid_hidden = False
