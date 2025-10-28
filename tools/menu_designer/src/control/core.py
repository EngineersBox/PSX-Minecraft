from typing import Optional
import pygame, pygame_gui
from src.preview import PREVIEW_SCALE_X, Preview, PreviewButton
from src.control.button import ControlButton
from src.control.horizontal_slider import ControlHorizontalSlider
from src.control.label import ControlLabel
from src.control.panel import ControlPanel
from src.control.resizing_panel import ControlResizingPanel
from src.control.selection_list import ControlSelectionList
from src.control.text_input import ControlTextInput

class Control:
    manager: pygame_gui.UIManager
    panel: ControlPanel
    preview: Preview

    buttons: dict[str, tuple[str, str]]

    create_panel: ControlResizingPanel
    modify_panel: ControlResizingPanel

    button_text_input: ControlTextInput
    buttons_list: ControlSelectionList
    width_slider_label: ControlLabel
    width_slider: ControlHorizontalSlider
    remove_button: ControlButton

    def __init__(self, manager: pygame_gui.UIManager, preview: Preview):
        self.manager = manager
        self.preview = preview
        self.buttons = {}
        self.create_panel_elements()

    def create_panel_elements(self):
        self.create_panel = ControlResizingPanel(
            self.preview.rect.width,
            0,
            200,
            0,
            self.manager
        )
        self.button_text_input = self.create_panel.add_element_offset(
            lambda width, y, container: ControlTextInput(
                0,
                y,
                width,
                30,
                self.manager,
                container
            )
        )
        self.create_panel.add_element_offset(
            lambda width, y, container: ControlButton(
                0,
                y,
                width,
                30,
                "Add Button",
                self.manager,
                container,
                self._add_button
            )
        )
        self.modify_panel = ControlResizingPanel(
            self.preview.rect.width,
            int(self.create_panel.y_offset + 5),
            200,
            0,
            self.manager
        )
        self.modify_panel.add_element_offset(
            lambda width, y, container: ControlLabel(
                0,
                y,
                width,
                30,
                "Buttons",
                self.manager,
                container
            )
        )
        self.buttons_list = self.modify_panel.add_element_offset(
            lambda width, y, container: ControlSelectionList(
                0,
                y,
                width,
                200,
                [],
                self.manager,
                container
            )
        )
        self.width_slider_label = self.modify_panel.add_element_offset(
            lambda width, y, container: ControlLabel(
                0,
                y,
                width,
                15,
                "",
                self.manager,
                container
            )
        )
        self.width_slider = self.modify_panel.add_element_offset(
            lambda width, y, container: ControlHorizontalSlider(
                0,
                y,
                width,
                30,
                (0, 320 * PREVIEW_SCALE_X),
                PREVIEW_SCALE_X,
                self.manager,
                container
            )
        )
        self.width_slider.disable()
        self.remove_button = self.modify_panel.add_element_offset(
            lambda width, y, container: ControlButton(
                0,
                y,
                width,
                30,
                "Remove Button",
                self.manager,
                container,
                self._remove_button
            )
        )
        self.remove_button.disable()

    def update(self, time_delta: float):
        self.create_panel.update(time_delta)
        self.modify_panel.update(time_delta)

    def process_event(self, event: pygame.Event):
        self.create_panel.process_event(event)
        self.modify_panel.process_event(event)
        if (event.type == pygame_gui.UI_SELECTION_LIST_NEW_SELECTION
            and event.ui_element == self.buttons_list.selection_list):
            self.width_slider.enable()
            self.remove_button.enable()
            selected = self._get_selected_button()
            if selected is not None:
                width = selected.get_width()
                self.width_slider.set_value(width)
                self.width_slider_label.label.set_text(f"{width}")
        if (event.type == pygame_gui.UI_SELECTION_LIST_DROPPED_SELECTION
            and event.ui_element == self.buttons_list.selection_list):
            self.width_slider.set_value(-1)
            self.width_slider.disable()
            self.remove_button.disable()
        if (event.type == pygame_gui.UI_HORIZONTAL_SLIDER_MOVED
            and event.ui_element == self.width_slider.slider):
            selected = self._get_selected_button()
            if selected is not None:
                width = self.width_slider.get_value()
                selected.set_width(width)
                self.width_slider_label.label.set_text(f"{width}")

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
        self.remove_button.disable()
        self.width_slider.disable()
        self.width_slider_label.label.set_text("")

    def _get_selected_button(self) -> Optional[PreviewButton]:
        selected = self.buttons_list.selection_list.get_single_selection(include_object_id=True)
        if selected == None:
            return None
        return self.preview.get_button(selected[1])
