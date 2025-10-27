from typing import Callable
import pygame, pygame_gui
from src.preview import Preview

class ControlButton:
    rect: pygame.Rect
    button: pygame_gui.elements.UIButton

    def __init__(
        self,
        x: int,
        y: int,
        width: int,
        height: int,
        text: str,
        manager: pygame_gui.UIManager,
        control_container: pygame_gui.elements.UIPanel,
        command: Callable
    ):
        self.rect = pygame.Rect(
            x,
            y,
            width,
            height
        )
        self.button = pygame_gui.elements.UIButton(
            relative_rect=self.rect,
            text=text,
            manager=manager,
            container=control_container,
            command=command
        )

class ControlTextInput():
    rect: pygame.Rect
    input: pygame_gui.elements.UITextEntryLine

    def __init__(
        self,
        x: int,
        y: int,
        width: int,
        height: int,
        manager: pygame_gui.UIManager,
        control_container: pygame_gui.elements.UIPanel
    ):
        self.rect = pygame.Rect(
            x,
            y,
            width,
            height
        )
        self.input = pygame_gui.elements.UITextEntryLine(
            relative_rect=self.rect,
            placeholder_text="Button text...",
            manager=manager,
            container=control_container
        )

class ControlForm:
    rect: pygame.Rect
    form: pygame_gui.elements.UIForm
    on_submit: Callable

    def __init__(
        self,
        x: int,
        y: int,
        width: int,
        height: int,
        questionnaire: dict,
        on_submit: Callable,
        manager: pygame_gui.UIManager,
        control_container: pygame_gui.elements.UIPanel
    ):
        self.rect = pygame.Rect(
            x,
            y,
            width,
            height
        )
        self.form = pygame_gui.elements.UIForm(
            relative_rect=self.rect,
            questionnaire=questionnaire,
            manager=manager,
            container=control_container
        )
        self.on_submit = on_submit
    
    def process_event(self, event: pygame.Event):
        if (event.type == pygame_gui.UI_FORM_SUBMITTED
            and event.ui_element == self.form):
            self.on_submit()

class Control:
    manager: pygame_gui.UIManager
    rect: pygame.Rect
    surface: pygame.Surface
    container: pygame_gui.elements.UIPanel
    preview: Preview

    button_text_input: ControlTextInput
    add_button_form: ControlForm

    def __init__(self, manager: pygame_gui.UIManager, preview: Preview):
        self.manager = manager
        self.rect = pygame.Rect(
            preview.rect.width,
            0,
            200,
            preview.rect.height
        )
        self.surface = pygame.Surface((self.rect.width, self.rect.height))
        self.surface.fill(pygame.Color("#AFAFAF"))
        self.container = pygame_gui.elements.UIPanel(relative_rect=self.rect)
        self.preview = preview
        self.create_panel_elements()

    def create_panel_elements(self):
        self.button_text_input = ControlTextInput(
            0,
            0,
            199,
            60,
            self.manager,
            self.container
        )
        self.add_button_form = ControlForm(
            0,
            0,
            199,
            150,
            {
                "button_text": self.button_text_input.input
            },
            self._add_button,
            self.manager,
            self.container
        )

    def update(self, time_delta: float):
        self.add_button_form.form.update(time_delta)

    def process_event(self, event: pygame.Event):
        self.add_button_form.process_event(event)

    def _add_button(self):
        text = self.button_text_input.input.get_text()
        if len(text) == 0:
            return
        self.preview.add_button(text)
