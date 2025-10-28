from typing import Optional
import pygame, pygame_gui
from src.codegen.button import gen_button_html_code
from src.control.button import ControlButton
from src.control.checkbox import ControlCheckbox
from src.control.horizontal_slider import ControlHorizontalSlider
from src.control.label import ControlLabel
from src.control.panel import ControlPanel
from src.control.resizing_panel import ControlResizingPanel
from src.control.selection_list import ControlSelectionList
from src.control.text_box import ControlTextBox
from src.control.text_input import ControlTextInput
from src.preview import Preview, PreviewButton

class Control:
    manager: pygame_gui.UIManager
    panel: ControlPanel
    preview: Preview

    buttons: dict[str, tuple[str, str]]

    create_panel: ControlResizingPanel
    modify_panel: ControlResizingPanel

    button_text_input: ControlTextInput
    buttons_list: ControlSelectionList
    pos_label: ControlLabel
    width_label: ControlLabel
    width_slider: ControlHorizontalSlider
    button_disabled_checkbox: ControlCheckbox
    remove_button: ControlButton
    view_button_code: ControlButton
    view_all_button_code: ControlButton

    code_panel: ControlPanel
    code_view: ControlTextBox
    code_close: ControlButton

    def __init__(self, manager: pygame_gui.UIManager, preview: Preview):
        self.manager = manager
        self.preview = preview
        self.buttons = {}
        self.create_panel_elements()

    def create_panel_elements(self):
        self.create_panel = ControlResizingPanel(
            self.preview.rect.width,
            0,
            pygame.display.get_window_size()[0] - self.preview.rect.width,
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
            int(self.create_panel.y_offset + 20),
            pygame.display.get_window_size()[0] - self.preview.rect.width,
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
        self.pos_label = self.modify_panel.add_element_offset(
            lambda width, y, container: ControlLabel(
                0,
                y,
                width,
                15,
                "X: N/A Y: N/A",
                self.manager,
                container
            )
        )
        self.width_label = self.modify_panel.add_element_offset(
            lambda width, y, container: ControlLabel(
                0,
                y,
                width,
                15,
                "Width: N/A",
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
                (1, 320),
                1,
                self.manager,
                container
            )
        )
        self.width_slider.disable()
        self.button_disabled_checkbox = self.modify_panel.add_element_offset(
            lambda _, y, container: ControlCheckbox(
                0,
                y,
                20,
                20,
                "Disabled State",
                self.manager,
                container,
                self._set_button_disabled
            ),
            process_event=True
        )
        self.button_disabled_checkbox.disable()
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
        self.view_button_code = self.modify_panel.add_element_offset(
            lambda width, y, container: ControlButton(
                0,
                y,
                width,
                30,
                "View Button Code",
                self.manager,
                container,
                self._open_selected_button_code_panel
            )
        )
        self.view_button_code.disable()
        self.view_all_button_code = self.modify_panel.add_element_offset(
            lambda width, y, container: ControlButton(
                0,
                y,
                width,
                30,
                "View All Buttons Code",
                self.manager,
                container,
                self._open_all_button_code_panel
            )
        )
        self.view_all_button_code.disable()

    def update(self, time_delta: float):
        self.create_panel.update(time_delta)
        self.modify_panel.update(time_delta)
        selected = self._get_selected_button()
        if selected is not None:
            pos = selected.get_pos()
            self.pos_label.label.set_text(f"X: {pos[0]} Y: {pos[1]}")

    def process_event(self, event: pygame.Event):
        self.create_panel.process_event(event)
        self.modify_panel.process_event(event)
        if (event.type == pygame_gui.UI_SELECTION_LIST_NEW_SELECTION
            and event.ui_element == self.buttons_list.selection_list):
            self.width_slider.enable()
            self.remove_button.enable()
            self.button_disabled_checkbox.enable()
            self.view_button_code.enable()
            selected = self._get_selected_button()
            if selected is not None:
                width = selected.get_width()
                self.width_slider.set_value(width)
                self.width_label.label.set_text(f"Width: {width}")
        if (event.type == pygame_gui.UI_SELECTION_LIST_DROPPED_SELECTION
            and event.ui_element == self.buttons_list.selection_list):
            self._reset_selection_controls()
        if (event.type == pygame_gui.UI_HORIZONTAL_SLIDER_MOVED
            and event.ui_element == self.width_slider.slider):
            selected = self._get_selected_button()
            if selected is not None:
                width = self.width_slider.get_value()
                selected.set_width(width)
                self.width_label.label.set_text(f"Width: {width}")

    def _add_button(self):
        text = self.button_text_input.input.get_text()
        if len(text) == 0:
            return
        button_id = self.preview.add_button(text)
        self.buttons[button_id] = (text, button_id)
        self.buttons_list.selection_list.set_item_list(list(self.buttons.values()))
        self.button_text_input.input.set_text("")
        self.view_all_button_code.enable()

    def _remove_button(self):
        selected = self.buttons_list.selection_list.get_single_selection(include_object_id=True)
        if selected == None:
            return
        self.buttons.pop(selected[1])
        self.preview.remove_button(selected[1])
        self.buttons_list.selection_list.set_item_list(list(self.buttons.values()))
        self._reset_selection_controls()

    def _get_selected_button(self) -> Optional[PreviewButton]:
        selected = self.buttons_list.selection_list.get_single_selection(include_object_id=True)
        if selected == None:
            return None
        return self.preview.get_button(selected[1])

    def _set_button_disabled(self, disabled: bool):
        selected = self._get_selected_button()
        if selected == None:
            return
        selected.set_disabled(disabled)

    def _reset_selection_controls(self):
        self.width_slider.set_value(-1)
        self.remove_button.disable()
        self.width_slider.disable()
        self.width_label.label.set_text("Width: N/A")
        self.pos_label.label.set_text("X: N/A Y: N/A")
        self.button_disabled_checkbox.disable()
        self.view_button_code.disable()
        if len(self.buttons) == 0:
            self.view_all_button_code.disable()

    def _close_code_panel(self):
        self.code_view.kill()
        self.code_close.kill()
        self.code_panel.kill()
        self.preview.show()
        self.create_panel.show()
        self.modify_panel.show()

    def _open_code_panel(self):
        self.preview.hide()
        self.create_panel.hide()
        self.modify_panel.hide()
        self.code_panel = ControlPanel(
            0,
            0,
            pygame.display.get_window_size()[0],
            pygame.display.get_window_size()[1],
            self.manager
        )
        self.code_view = ControlTextBox(
            0,
            0,
            pygame.display.get_window_size()[0],
            pygame.display.get_window_size()[1] - 36,
            self.manager,
            self.code_panel.panel,
            background_colour=pygame.Color("#232323")
        )
        self.code_close = ControlButton(
            pygame.display.get_window_size()[0] - 106,
            pygame.display.get_window_size()[1] - 36,
            100,
            30,
            "Close",
            self.manager,
            self.code_panel.panel,
            self._close_code_panel
        )

    def _open_selected_button_code_panel(self):
        selected = self._get_selected_button()
        if selected == None:
            return
        self._open_code_panel()
        self.code_view.set_text(gen_button_html_code(selected))

    def _open_all_button_code_panel(self):
        code_chunks = []
        for button_id in self.buttons.keys():
            button = self.preview.get_button(button_id)
            if button == None:
                continue
            code_chunks.append(gen_button_html_code(button))
        self._open_code_panel()
        self.code_view.set_text("\n".join(code_chunks))
