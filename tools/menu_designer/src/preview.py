import pygame, pygame_gui, uuid

PREVIEW_SCALE_X = 3
PREVIEW_SCALE_Y = 3

class PreviewButton:
    rect: pygame.Rect
    image: pygame_gui.elements.UIImage
    label: pygame_gui.elements.UILabel

    def __init__(
        self,
        x: int,
        y: int,
        width: int,
        text: str,
        image: pygame.Surface,
        manager: pygame_gui.UIManager,
        preview_container: pygame_gui.elements.UIPanel
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
        for button in self.game_buttons.values():
            if (button.image.hover_point(pos[0], pos[1])):
                return button
        return None

    def process_event(self, event: pygame.Event):
        if (event.type == pygame.MOUSEBUTTONDOWN):
            self.held_button = self._get_intersected_button(pygame.mouse.get_pos())
            self.mouse_pos = pygame.mouse.get_pos()
        elif (event.type == pygame.MOUSEBUTTONUP):
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
