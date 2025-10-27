from typing import Callable
import pygame, pygame_gui
from src.preview import Preview

class ControlLabel:
    rect: pygame.Rect
    label: pygame_gui.elements.UILabel

    def __init__(
        self,
        x: int,
        y: int,
        width: int,
        height: int,
        text: str,
        manager: pygame_gui.UIManager,
        control_container: pygame_gui.elements.UIPanel
    ):
        self.rect = pygame.Rect(
            x,
            y,
            width,
            height
        )
        self.label = pygame_gui.elements.UILabel(
            relative_rect=self.rect,
            text=text,
            manager=manager,
            container=control_container
        )

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

class ControlSelectionList:
    rect: pygame.Rect
    selection_list: pygame_gui.elements.UISelectionList

    def __init__(
        self,
        x: int,
        y: int,
        width: int,
        height: int,
        item_list: list[str | tuple[str, str]],
        manager: pygame_gui.UIManager,
        control_container: pygame_gui.elements.UIPanel
    ):
        self.rect = pygame.Rect(
            x,
            y,
            width,
            height
        )
        self.selection_list = pygame_gui.elements.UISelectionList(
            relative_rect=self.rect,
            item_list=item_list,
            allow_multi_select=False,
            manager=manager,
            container=control_container
        )

class Control:
    manager: pygame_gui.UIManager
    rect: pygame.Rect
    surface: pygame.Surface
    container: pygame_gui.elements.UIPanel
    preview: Preview

    buttons: dict[str, tuple[str, str]]

    button_text_input: ControlTextInput
    create_button: ControlButton
    buttons_list_label: ControlLabel
    buttons_list: ControlSelectionList
    delete_button: ControlButton

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
        self.buttons = {}
        self.create_panel_elements()

    def create_panel_elements(self):
        y_offset = 0
        self.button_text_input = ControlTextInput(
            0,
            y_offset,
            199,
            30,
            self.manager,
            self.container
        )
        y_offset += self.button_text_input.rect.height
        self.create_button = ControlButton(
            0,
            y_offset,
            199,
            30,
            "Add Button",
            self.manager,
            self.container,
            self._add_button
        )
        y_offset += self.create_button.rect.height + 5
        self.buttons_list_label = ControlLabel(
            0,
            y_offset,
            199,
            30,
            "Buttons",
            self.manager,
            self.container
        )
        y_offset += self.buttons_list_label.rect.height
        self.buttons_list = ControlSelectionList(
            0,
            y_offset,
            199,
            200,
            [],
            self.manager,
            self.container
        )
        y_offset += self.buttons_list.rect.height
        self.delete_button = ControlButton(
            0,
            y_offset,
            199,
            30,
            "Remove Button",
            self.manager,
            self.container,
            self._remove_button
        )

    def update(self, time_delta: float):
        pass

    def process_event(self, event: pygame.Event):
        pass

    def _add_button(self):
        text = self.button_text_input.input.get_text()
        if len(text) == 0:
            return
        button_id = self.preview.add_button(text)
        self.buttons[button_id] = (text, button_id)
        self.buttons_list.selection_list.set_item_list(list(self.buttons.values()))
        self.button_text_input.input.set_text("")

    def _remove_button(self):
        selected = self.buttons_list.selection_list.get_single_selection(include_object_id=True)
        if selected == None:
            return
        self.buttons.pop(selected[1])
        self.preview.remove_button(selected[1])
        self.buttons_list.selection_list.set_item_list(list(self.buttons.values()))

