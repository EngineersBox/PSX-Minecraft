import pygame, pygame_gui

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
                ((rel_rect.x + move[0]) // PREVIEW_SCALE_X) * PREVIEW_SCALE_X,
                ((rel_rect.y + move[1]) // PREVIEW_SCALE_Y) * PREVIEW_SCALE_Y
            ))
            rel_rect = self.label.get_relative_rect()
            self.label.set_relative_position((
                ((rel_rect.x + move[0]) // PREVIEW_SCALE_X) * PREVIEW_SCALE_X,
                ((rel_rect.y + move[1]) // PREVIEW_SCALE_Y) * PREVIEW_SCALE_Y
            ))

class Preview:
    manager: pygame_gui.UIManager
    button_image: pygame.Surface
    game_buttons: list[PreviewButton]
    rect: pygame.Rect
    surface: pygame.Surface
    container: pygame_gui.elements.UIPanel

    held_button: PreviewButton | None

    def __init__(self, manager: pygame_gui.UIManager):
        self.manager = manager
        self.button_image = pygame.image.load("assets/button.png")
        self.game_buttons = []
        self.rect = pygame.Rect(0, 0, 320 * PREVIEW_SCALE_X, 240 * PREVIEW_SCALE_Y)
        self.surface = pygame.Surface((self.rect.width, self.rect.height))
        self.surface.fill(pygame.Color("#AFAFAF"))
        self.container = pygame_gui.elements.UIPanel(relative_rect=self.rect)
        self.held_button = None

    def update(self, time_delta: float):
        for button in self.game_buttons:
            button.update(time_delta)
        if (self.held_button is not None):
            move = pygame.mouse.get_rel()
            self.held_button.move(move, self.container.get_relative_rect())

    def _get_intersected_button(self, pos: tuple[int, int]) -> PreviewButton | None:
        for button in self.game_buttons:
            if (button.image.get_abs_rect().collidepoint(pos[0], pos[1])):
                return button
        return None

    def process_event(self, event: pygame.Event):
        if (event.type == pygame.MOUSEBUTTONDOWN):
            self.held_button = self._get_intersected_button(pygame.mouse.get_pos())
        elif (event.type == pygame.MOUSEBUTTONUP):
            self.held_button = None
        for button in self.game_buttons:
            button.process_event(event)

    def add_button(self, text: str):
        self.game_buttons.append(PreviewButton(
            0,
            0,
            200,
            text,
            self.button_image,
            self.manager,
            self.container
        ))
