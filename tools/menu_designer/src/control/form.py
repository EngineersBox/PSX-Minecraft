import pygame, pygame_gui
from typing import Callable
from src.control.base import ControlBase

class ControlForm(ControlBase):
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
        control_container: pygame_gui.core.IContainerLikeInterface
    ):
        super().__init__(
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

