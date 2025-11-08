from pathlib import Path
from typing import Callable, Optional, cast
import pygame, pygame_gui
from src.codegen.gen import gen_html_code
from src.control.button import ControlButton
from src.control.checkbox import ControlCheckbox
from src.control.colour_picker import ControlColourPicker
from src.control.drop_down import ControlDropDown
from src.control.file_dialog import ControlFileDialog
from src.control.horizontal_slider import ControlHorizontalSlider
from src.control.label import ControlLabel
from src.control.panel import ControlPanel
from src.control.resizing_panel import ControlResizingPanel
from src.control.selection_list import ControlSelectionList
from src.control.textures import Textures
from src.control.text_box import ControlTextBox
from src.control.text_input import ControlTextInput, number_only_validator
from src.control.window import ControlWindow, ControlWindowClosedCommand
from src.preview.core import Preview
from src.preview.const import PREVIEW_SCALE_Y, PREVIEW_SIZE_X, PREVIEW_SIZE_Y, PREVIEW_SCALE_X
from src.preview.background import PreviewBackground
from src.preview.button import PreviewButton
from src.preview.element import PreviewElement

type Elements = dict[str, tuple[str, str]]

class ButtonModifiersPanel(ControlResizingPanel):
    button_state_dropdown: ControlDropDown

    _get_selected_element: Callable[[], Optional[PreviewElement]]

    def __init__(
        self,
        y: int,
        get_selected_element: Callable[[], Optional[PreviewElement]],
        manager: pygame_gui.UIManager,
        parent_container: Optional[pygame_gui.core.IContainerLikeInterface] = None
    ):
        super().__init__(
            PREVIEW_SIZE_X * PREVIEW_SCALE_X,
            y,
            pygame.display.get_window_size()[0] - (PREVIEW_SIZE_X * PREVIEW_SCALE_X),
            0,
            manager,
            parent_container
        )
        self.button_state_dropdown = self.add_element_offset(
            lambda width, y, container: ControlDropDown(
                0,
                y,
                width,
                30,
                [
                    ("Normal", "normal"),
                    ("Disabled", "disabled"),
                    ("Actve", "active")
                ],
                self._set_button_state,
                manager,
                container
            ),
            process_event=True
        )
        self._get_selected_element = get_selected_element

    def _set_button_state(self, state: tuple[str, str]):
        selected = self._get_selected_element()
        if selected == None or type(selected) != PreviewButton:
            return
        selected.set_state(state)

    def reset_controls(self):
        self.button_state_dropdown.disable()

    def update_from_element(self, button: PreviewButton):
        self.button_state_dropdown.drop_down.selected_option = button._state

class BackgroundModifiersPanel(ControlResizingPanel):
    direct_blit_checkbox: ControlCheckbox
    texture_dropdown: ControlDropDown
    tile_x_drop_down: ControlDropDown
    tile_y_drop_down: ControlDropDown
    pos_u_label: ControlLabel
    pos_u_slider: ControlHorizontalSlider
    pos_v_label: ControlLabel
    pos_v_slider: ControlHorizontalSlider
    tint_label: ControlLabel
    tint_picker_open_button: ControlButton
    tint_colour_picker: ControlColourPicker

    _textures: Textures
    _preview: Preview
    _get_selected_element: Callable[[], Optional[PreviewElement]]

    def __init__(
        self,
        y: int,
        get_selected_element: Callable[[], Optional[PreviewElement]],
        textures: Textures,
        preview: Preview,
        manager: pygame_gui.UIManager,
        parent_container: Optional[pygame_gui.core.IContainerLikeInterface] = None
    ):
        super().__init__(
            PREVIEW_SIZE_X * PREVIEW_SCALE_X,
            y,
            pygame.display.get_window_size()[0] - (PREVIEW_SIZE_X * PREVIEW_SCALE_X),
            0,
            manager,
            parent_container
        )
        self.direct_blit_checkbox = self.add_element_offset(
            lambda _, y, container: ControlCheckbox(
                0,
                y,
                30,
                30,
                "Direct Blit",
                manager,
                container,
                self._direct_blit_toggled
            ),
            process_event=True
        )
        self.add_element_offset(
            lambda width, y, container: ControlDropDown(
                0,
                y,
                width,
                30,
                list(self._textures.mappings.keys()),
                self._texture_selected,
                manager,
                container
            ),
            process_event=True
        )
        self.tile_x_drop_down = self.add_element_offset(
            lambda width, y, container: ControlDropDown(
                0,
                y,
                width,
                30,
                ["8", "16", "32", "64", "128", "256"],
                self._tile_x_selected,
                manager,
                container,
                starting_option="256"
            ),
            process_event=True
        )
        self.tile_y_drop_down = self.add_element_offset(
            lambda width, y, container: ControlDropDown(
                0,
                y,
                width,
                30,
                ["8", "16", "32", "64", "128", "256"],
                self._tile_y_selected,
                manager,
                container,
                starting_option="256"
            ),
            process_event=True
        )
        self.pos_u_label = self.add_element_offset(
            lambda width, y, container: ControlLabel(
                0,
                y,
                width,
                30,
                "U: N/A",
                manager,
                container
            )
        )
        self.pos_u_slider = self.add_element_offset(
            lambda width, y, container: ControlHorizontalSlider(
                0,
                y,
                width,
                30,
                (0, 255),
                1,
                manager,
                container,
                self._pos_u_updated
            ),
            process_event=True
        )
        self.pos_v_label = self.add_element_offset(
            lambda width, y, container: ControlLabel(
                0,
                y,
                width,
                30,
                "V: N/A",
                manager,
                container
            )
        )
        self.pos_v_slider = self.add_element_offset(
            lambda width, y, container: ControlHorizontalSlider(
                0,
                y,
                width,
                30,
                (0, 255),
                1,
                manager,
                container,
                self._pos_v_updated
            ),
            process_event=True
        )
        self.tint_label = self.add_element_offset(
            lambda width, y, container: ControlLabel(
                0,
                y,
                width,
                30,
                "#808080",
                manager,
                container
            )
        )
        self.tint_picker_open_button = self.add_element_offset(
            lambda width, y, container: ControlButton(
                0,
                y,
                width,
                30,
                "Select Tint",
                manager,
                container,
                self._open_tint_colour_picker
            ),
            process_event=True
        )
        self.tint_colour_picker = self.add_element(
            ControlColourPicker(
                0,
                0,
                200,
                200,
                "Tint Colour",
                self._tint_colour_selected,
                self._close_tint_colour_picker,
                manager
            ),
            process_event=True,
            update=True
        )
        self.tint_label.set_background_colour(self.tint_colour_picker.get_colour())
        self._textures = textures
        self._get_selected_element = get_selected_element
        self._preview = preview

    def reset_controls(self):
        pass

    def update_from_element(self, background: PreviewBackground):
        self.direct_blit_checkbox.set_state(background.get_direct_blit())
        self.pos_u_label.set_text(f"U: {background.get_pos_u()}")
        self.pos_u_slider.slider.value_range = (0, background.source_image.width - 1)
        self.pos_u_slider.set_value(background.get_pos_u())
        self.pos_v_label.set_text(f"V: {background.get_pos_v()}")
        self.pos_v_slider.slider.value_range = (0, background.source_image.height - 1)
        self.pos_v_slider.set_value(background.get_pos_v())
        self.tile_x_drop_down.drop_down.selected_option = (
            f"{background.get_tile_x()}",
            f"{background.get_tile_x()}"
        )
        self.tile_y_drop_down.drop_down.selected_option = (
            f"{background.get_tile_y()}",
            f"{background.get_tile_y()}"
        )
        self.texture_dropdown.drop_down.selected_option = (
            background.get_tim_name(),
            background.get_tim_name()
        )

    def _texture_selected(self, texture: tuple[str, str]):
        element = self._get_selected_element()
        if (element == None or type(element) != PreviewBackground):
            return
        background = cast(PreviewBackground, element)
        self.pos_u_label.set_text(f"U: {background.get_pos_u()}")
        self.pos_u_slider.slider.value_range = (0, background.source_image.width - 1)
        self.pos_u_slider.set_value(background.get_pos_u())
        self.pos_v_label.set_text(f"V: {background.get_pos_v()}")
        self.pos_v_slider.slider.value_range = (0, background.source_image.height - 1)
        self.pos_v_slider.set_value(background.get_pos_v())
        background.set_pos_u(0)
        background.set_pos_v(0)
        background.set_texture(texture[0], self._textures.mappings[texture[0]])

    def _tile_x_selected(self, choice: tuple[str, str]):
        element = self._get_selected_element()
        if (element == None or type(element) != PreviewBackground):
            return
        background = cast(PreviewBackground, element)
        background.set_tile_x(int(choice[0]))

    def _tile_y_selected(self, choice: tuple[str, str]):
        element = self._get_selected_element()
        if (element == None or type(element) != PreviewBackground):
            return
        background = cast(PreviewBackground, element)
        background.set_tile_y(int(choice[0]))

    def _pos_u_updated(self, pos_u: int):
        element = self._get_selected_element()
        if (element == None or type(element) != PreviewBackground):
            return
        background = cast(PreviewBackground, element)
        background.set_pos_u(pos_u)
        self.pos_u_label.set_text(f"U: {pos_u}")

    def _pos_v_updated(self, pos_v: int):
        element = self._get_selected_element()
        if (element == None or type(element) != PreviewBackground):
            return
        background = cast(PreviewBackground, element)
        background.set_pos_v(pos_v)
        self.pos_v_label.set_text(f"V: {pos_v}")

    def _direct_blit_toggled(self, state: bool):
        element = self._get_selected_element()
        if (element == None or type(element) != PreviewBackground):
            return
        background = cast(PreviewBackground, element)
        background.set_direct_blit(state)
        if state:
            self.pos_u_slider.disable()
            self.pos_v_slider.disable()
            self.tile_x_drop_down.disable()
            self.tile_y_drop_down.disable()
        else:
            self.pos_u_slider.enable()
            self.pos_v_slider.enable()
            self.tile_x_drop_down.enable()
            self.tile_y_drop_down.enable()

    def _open_tint_colour_picker(self):
        element = self._get_selected_element()
        if (element == None or type(element) != PreviewBackground):
            return
        background = cast(PreviewBackground, element)
        self.tint_colour_picker.open()
        self.tint_colour_picker.set_colour(background.get_tint())
        self._preview.disable()

    def _close_tint_colour_picker(self):
        self.tint_colour_picker.close()
        self._preview.enable()

    def _tint_colour_selected(self, colour: pygame.Color):
        self._preview.enable()
        element = self._get_selected_element()
        if (element == None or type(element) != PreviewBackground):
            return
        background = cast(PreviewBackground, element)
        background.set_tint(colour)
        self.tint_label.set_background_colour(colour)
        self.tint_label.set_text(colour.hex)

class CodePanel(ControlPanel):
    code_view: ControlTextBox
    close_button: ControlButton
    font_drop_down: ControlDropDown
    
    _elements: list[PreviewElement]
    _close_command: Callable[[], None]

    def __init__(
        self,
        elements: list[PreviewElement],
        close_command: Callable[[], None],
        manager: pygame_gui.UIManager,
        parent_container: Optional[pygame_gui.core.IContainerLikeInterface] = None
    ):
        super().__init__(
            0,
            0,
            pygame.display.get_window_size()[0],
            pygame.display.get_window_size()[1],
            manager,
            parent_container
        )
        self.code_view = ControlTextBox(
            0,
            0,
            pygame.display.get_window_size()[0],
            pygame.display.get_window_size()[1] - 36,
            manager,
            self.panel,
            background_colour=pygame.Color("#232323")
        )
        code_blocks = []
        for element in elements:
            code_blocks.append(gen_html_code(element))
        self.code_view.set_text("\n".join(code_blocks))
        self.close_button = self.add_element(
            ControlButton(
                pygame.display.get_window_size()[0] - 106,
                pygame.display.get_window_size()[1] - 36,
                100,
                30,
                "Close",
                manager,
                self.panel,
                close_command
            ),
            process_event=True
        )
        self.font_drop_down = self.add_element(
            ControlDropDown(
                0,
                pygame.display.get_window_size()[1] - 36,
                100,
                30,
                [(font, font) for font in pygame.font.get_fonts()],
                self._update_code_view_font,
                manager,
                self.panel
            ),
            process_event=True
        )
        self._elements = elements
        self._close_command = close_command

    def _update_code_view_font(self, selected_font: tuple[str, str]):
        code_blocks = []
        for element in self._elements:
            code_blocks.append(gen_html_code(element, selected_font[1]))
        self.code_view.set_text("\n".join(code_blocks))

    def kill(self):
        super().kill()
        self.code_view.kill()
        self.close_button.kill()
        self.font_drop_down.kill()

class ElementsPanel(ControlResizingPanel):
    elements_list: ControlSelectionList

    remove_element_button: ControlButton
    view_element_code_button: ControlButton
    view_all_code_button: ControlButton

    code_panel: Optional[CodePanel]

    pos_label: ControlLabel
    width_label: ControlLabel
    width_slider: ControlHorizontalSlider
    height_label: ControlLabel
    height_slider: ControlHorizontalSlider

    button_modifiers_panel: ButtonModifiersPanel
    background_modifiers_panel: BackgroundModifiersPanel

    _manager: pygame_gui.UIManager
    _elements: Elements
    _preview: Preview

    def __init__(
        self,
        y: int,
        elements: Elements,
        textures: Textures,
        preview: Preview,
        manager: pygame_gui.UIManager,
        parent_container: Optional[pygame_gui.core.IContainerLikeInterface] = None
    ):
        super().__init__(
            PREVIEW_SIZE_X * PREVIEW_SCALE_X,
            y,
            pygame.display.get_window_size()[0] - (PREVIEW_SIZE_X * PREVIEW_SCALE_X),
            0,
            manager,
            parent_container
        )
        self.elements_list = self.add_element_offset(
            lambda width, y, container: ControlSelectionList(
                0,
                y,
                width,
                200,
                [],
                self._selected_element,
                self._dropped_element,
                manager,
                container
            ),
            process_event=True
        )
        self.remove_element_button = self.add_element_offset(
            lambda width, y, container: ControlButton(
                0,
                y,
                width,
                30,
                "Remove Element",
                manager,
                container,
                self._remove_element
            ),
            process_event=True
        )
        self.view_element_code_button = self.add_element_offset(
            lambda width, y, container: ControlButton(
                0,
                y,
                width,
                30,
                "View Element Code",
                manager,
                container,
                self._open_selected_element_code_panel
            ),
            process_event=True
        )
        self.view_all_code_button = self.add_element_offset(
            lambda width, y, container: ControlButton(
                0,
                y,
                width,
                30,
                "View All Code",
                manager,
                container,
                self._open_all_elements_code_panel
            ),
            process_event=True
        )
        self.pos_x_label = self.add_element_offset(
            lambda width, y, container: ControlLabel(
                0,
                y,
                width,
                30,
                "X: N/A Y: N/A",
                manager,
                container
            )
        )
        self.width_label = self.add_element_offset(
            lambda width, y, container: ControlLabel(
                0,
                y,
                width,
                30,
                "Width: N/A",
                manager,
                container
            )
        )
        self.width_slider = self.add_element_offset(
            lambda width, y, container: ControlHorizontalSlider(
                0,
                y,
                width,
                30,
                (0, PREVIEW_SIZE_X),
                1,
                manager,
                container,
                self._width_slider_moved
            ),
            process_event=True
        )
        self.height_label = self.add_element_offset(
            lambda width, y, container: ControlLabel(
                0,
                y,
                width,
                30,
                "Height: N/A",
                manager,
                container
            )
        )
        self.height_slider = self.add_element_offset(
            lambda width, y, container: ControlHorizontalSlider(
                0,
                y,
                width,
                30,
                (0, PREVIEW_SIZE_Y),
                1,
                manager,
                container,
                self._height_slider_moved
            ),
            process_event=True
        )
        previous_y = self.y_offset
        self.background_modifiers_panel = self.add_element(
            BackgroundModifiersPanel(
                self.rect.y + self.y_offset + 1,
                self.get_selected_element,
                textures,
                preview,
                manager,
                parent_container
            ),
            process_event=True
        )
        self.background_modifiers_panel.disable()
        self.background_modifiers_panel.hide()
        bg_y_offset = self.y_offset
        self.y_offset = previous_y
        self.button_modifiers_panel = self.add_element(
            ButtonModifiersPanel(
                self.rect.y + self.y_offset + 1,
                self.get_selected_element,
                manager,
                parent_container
            ),
            process_event=True
        )
        self.button_modifiers_panel.disable()
        self.button_modifiers_panel.hide()
        button_y_offset = self.y_offset
        self.y_offset = max(bg_y_offset, button_y_offset)
        self.panel.set_dimensions((
            self.panel.get_abs_rect().width,
            self.panel.get_abs_rect().height + self.y_offset
        ))
        self.code_panel = None
        self._manager = manager
        self._elements = elements
        self._preview = preview
        self.reset_controls()

    def process_event(self, event: pygame.Event):
        super().process_event(event)
        if self.code_panel is not None:
            self.code_panel.process_event(event)
        selected = self.get_selected_element()
        if selected == None:
            return
        pos = selected.get_pos()
        self.pos_x_label.set_text(f"X: {pos[0]} Y: {pos[1]}")

    def reset_controls(self):
        self.remove_element_button.disable()
        self.view_element_code_button.disable()
        if len(self.elements_list.selection_list.item_list) > 0:
            self.view_all_code_button.enable()
        else:
            self.view_all_code_button.disable()
        self.pos_x_label.set_text("X: N/A Y: N/A")
        self.width_label.set_text("Width: N/A")
        self.width_slider.set_value(-1)
        self.width_slider.disable()
        self.height_label.set_text("Height: N/A")
        self.height_slider.set_value(-1)
        self.height_slider.disable()
        self.button_modifiers_panel.reset_controls()
        self.button_modifiers_panel.disable()
        self.button_modifiers_panel.hide()
        self.background_modifiers_panel.reset_controls()
        self.background_modifiers_panel.disable()
        self.background_modifiers_panel.hide()

    def get_selected_element(self) -> Optional[PreviewElement]:
        selected = self.elements_list.selection_list.get_single_selection(include_object_id=True)
        if selected == None:
            return None
        return self._preview.get_element(selected[1])

    def _selected_element(self, id: tuple[str, str]):
        self.enable()
        self.show()
        selected = self._preview.get_element(id[1])
        if selected == None:
            return
        self.remove_element_button.enable()
        self.view_element_code_button.enable()
        pos = selected.get_pos()
        self.pos_x_label.set_text(f"X: {pos[0]} Y: {pos[1]}")
        width = selected.get_width()
        self.width_slider.set_value(width)
        self.width_label.set_text(f"Width: {width}")
        self.width_slider.enable()
        height = selected.get_height()
        self.height_slider.set_value(height)
        self.height_label.set_text(f"Height: {height}")
        self.height_slider.enable()
        if type(selected) == PreviewButton:
            self.button_modifiers_panel.enable()
            self.button_modifiers_panel.show()
            self.button_modifiers_panel.update_from_element(cast(PreviewButton, selected))
            self.width_slider.slider.value_range = (0, PREVIEW_SIZE_X)
            self.height_slider.slider.value_range = (0, PREVIEW_SIZE_Y)
            self.background_modifiers_panel.disable()
            self.background_modifiers_panel.hide()
        elif type(selected) == PreviewBackground:
            self.background_modifiers_panel.enable()
            self.background_modifiers_panel.show()
            self.background_modifiers_panel.update_from_element(cast(PreviewBackground, selected))
            self.width_slider.slider.value_range = (0, 256)
            self.width_slider.slider.value_range = (0, PREVIEW_SIZE_Y)
            self.button_modifiers_panel.disable()
            self.button_modifiers_panel.hide()

    def _dropped_element(self):
        self.reset_controls()

    def _remove_element(self):
        selected = self.elements_list.selection_list.get_single_selection(include_object_id=True)
        if selected == None:
            return
        self._elements.pop(selected[1])
        self._preview.remove_element(selected[1])
        self.elements_list.selection_list.set_item_list(list(self._elements.values()))
        self.reset_controls()

    def _open_selected_element_code_panel(self):
        selected = self.get_selected_element()
        if selected == None:
            return
        self.code_panel = CodePanel(
            [selected],
            self._close_code_panel,
            self._manager
        )
        self._preview.hide()
        self.hide()
        self.disable()

    def _open_all_elements_code_panel(self):
        elements = []
        for element_id in self._elements.keys():
            element = self._preview.get_element(element_id)
            if element == None:
                continue
            elements.append(element)
        self.code_panel = CodePanel(
            elements,
            self._close_code_panel,
            self._manager
        )
        self._preview.hide()
        self.hide()
        self.disable()

    def _close_code_panel(self):
        if self.code_panel == None:
            return
        self.code_panel.kill()
        self.code_panel = None
        self._preview.show()
        self.show()
        self.enable()
        self.reset_controls()

    def _width_slider_moved(self, width: int):
        self.width_label.set_text(f"Width: {width}")
        selected = self.get_selected_element()
        if selected == None:
            return
        selected.set_width(width)

    def _height_slider_moved(self, height: int):
        self.height_label.set_text(f"Height: {height}")
        selected = self.get_selected_element()
        if selected == None:
            return
        selected.set_height(height)

class CreateButtonWindow(ControlWindow):
    label_text_input: ControlTextInput
    width_input: ControlTextInput
    height_input: ControlTextInput
    create_button: ControlButton

    _elements: Elements
    _elements_panel: ElementsPanel
    _preview: Preview

    def __init__(
        self,
        manager: pygame_gui.UIManager,
        closed_command: ControlWindowClosedCommand,
        elements: Elements,
        elements_panel: ElementsPanel,
        preview: Preview
    ):
        window_width = 300
        window_height = 200
        super().__init__(
            (pygame.display.get_window_size()[0] // 2) - (window_width // 2),
            (pygame.display.get_window_size()[1] // 2) - (window_height // 2),
            window_width,
            window_height,
            "Add Button",
            closed_command,
            manager,
            draggable=True,
            always_on_top=True
        )
        input_width = 200
        self.label_text_input = ControlTextInput(
            (window_width // 2) - (input_width // 2),
            30,
            input_width,
            30,
            manager,
            self.window
        )
        self.width_input = self.add_element(
            ControlTextInput(
                (window_width // 2) - 50,
                65,
                100,
                30,
                manager,
                self.window,
                number_only_validator
            ),
            process_event=True
        )
        self.height_input = self.add_element(
            ControlTextInput(
                (window_width // 2) - 50,
                100,
                100,
                30,
                manager,
                self.window,
                number_only_validator
            ),
            process_event=True
        )
        button_width = 70
        self.create_button = self.add_element(
            ControlButton(
                (window_width // 2) - (button_width // 2),
                135,
                button_width,
                30,
                "Create",
                manager,
                self.window,
                self._add_button
            ),
            process_event=True
        )
        self.create_button.disable()
        self._elements = elements
        self._elements_panel = elements_panel
        self._preview = preview

    def kill(self):
        self.label_text_input.kill()
        self.width_input.kill()
        self.height_input.kill()
        self.create_button.kill()
        super().kill()

    def process_event(self, event: pygame.Event):
        super().process_event(event)
        if (event.type == pygame_gui.UI_TEXT_ENTRY_CHANGED
            and event.ui_element == self.label_text_input.input):
            if len(self.label_text_input.get_text()) > 0:
                self.create_button.enable()
            else:
                self.create_button.disable()

    def _add_button(self):
        text = self.label_text_input.input.get_text()
        if len(text) == 0:
            return
        button_id = self._preview.add_button(text)
        self._elements[button_id] = (text, button_id)
        self._elements_panel.elements_list.selection_list.set_item_list(list(self._elements.values()))
        element = self._preview.get_element(button_id)
        if element == None:
            raise Exception("Should not happen")
        width_text = self.width_input.get_text()
        if width_text == None and len(width_text) > 0:
            width = int(width_text)
            element.set_width(width)
        height_text = self.height_input.get_text()
        if height_text == None and len(height_text) > 0:
            height = int(height_text)
            element.set_height(height)
        self._elements_panel.reset_controls()
        self._elements_panel.view_all_code_button.enable()
        self.closed_command()

class CreateBackgroundWindow(ControlWindow):
    texture_dropdown: ControlDropDown
    label_text_input: ControlTextInput
    width_label: ControlLabel
    width_input: ControlTextInput
    height_label: ControlLabel
    height_input: ControlTextInput
    create_button: ControlButton

    _manager: pygame_gui.UIManager
    _image: Optional[pygame.Surface]
    _textures: Textures
    _elements: Elements
    _elements_panel: ElementsPanel
    _preview: Preview

    def __init__(
        self,
        manager: pygame_gui.UIManager,
        closed_command: ControlWindowClosedCommand,
        elements: Elements,
        elements_panel: ElementsPanel,
        textures: Textures,
        preview: Preview
    ):
        window_width = 250
        window_height = 230
        super().__init__(
            (pygame.display.get_window_size()[0] // 2) - (window_width // 2),
            (pygame.display.get_window_size()[1] // 2) - (window_height // 2),
            window_width,
            window_height,
            "Add Background",
            closed_command,
            manager,
            draggable=True,
            always_on_top=True
        )
        y_offset = 5
        self.label_text_input = ControlTextInput(
            (window_width // 2) - 50,
            y_offset,
            100,
            30,
            manager,
            self.window,
            placeholder_text="Name..."
        )
        y_offset += self.label_text_input.rect.height
        self.width_label = ControlLabel(
            (window_width // 2) - 50 - 50,
            y_offset,
            50,
            30,
            "Width:",
            manager,
            self.window,
        )
        self.width_input = self.add_element(
            ControlTextInput(
                (window_width // 2) - 50,
                y_offset,
                100,
                30,
                manager,
                self.window,
                number_only_validator,
                initial_text="100"
            ),
            process_event=True
        )
        y_offset += self.width_input.rect.height
        self.height_label = ControlLabel(
            (window_width // 2) - 50 - 53,
            y_offset,
            50,
            30,
            "Height:",
            manager,
            self.window,
        )
        self.height_input = self.add_element(
            ControlTextInput(
                (window_width // 2) - 50,
                y_offset,
                100,
                30,
                manager,
                self.window,
                number_only_validator,
                initial_text="100"
            ),
            process_event=True
        )
        y_offset += self.height_input.rect.height
        self.texture_dropdown = self.add_element(
            ControlDropDown(
                (window_width // 2) - 55,
                y_offset,
                110,
                30,
                list(textures.mappings.keys()),
                self._texture_selected,
                manager,
                self.window
            ),
            process_event=True
        )
        y_offset += self.texture_dropdown.rect.height
        self.create_button = self.add_element(
            ControlButton(
                (window_width // 2) - 35,
                y_offset + 5,
                70,
                30,
                "Create",
                manager,
                self.window,
                self._add_background
            ),
            process_event=True
        )
        self._manager = manager
        self._textures = textures
        self._elements = elements
        self._elements_panel = elements_panel
        self._preview = preview
        self._image = None

    def process_event(self, event: pygame.Event):
        super().process_event(event)
        name = self.label_text_input.get_text()
        if (len(name) == 0 or self._image == None):
            self.create_button.disable()
        else:
            self.create_button.enable()


    def kill(self):
        self.width_input.kill()
        self.width_label.kill()
        self.height_input.kill()
        self.height_label.kill()
        self.create_button.kill()
        super().kill()

    def _texture_selected(self, _: tuple[str, str]):
        # TODO: Preview texture in window
        pass

    def _add_background(self):
        name = self.label_text_input.get_text()
        if (len(name) == 0 or self._image == None):
            return
        texture = self.texture_dropdown.drop_down.selected_option
        background_id = self._preview.add_background(
            name,
            texture[0],
            pygame.image.load(self._textures.mappings[texture[0]])
        )
        self._elements[background_id] = (name, background_id)
        self._elements_panel.elements_list.selection_list.set_item_list(list(self._elements.values()))
        element = self._preview.get_element(background_id)
        if element == None:
            raise Exception("Should not happen")
        width_text = self.width_input.get_text()
        if width_text == None and len(width_text) > 0:
            width = int(width_text)
            element.set_width(width)
        height_text = self.height_input.get_text()
        if height_text == None and len(height_text) > 0:
            height = int(height_text)
            element.set_height(height)
        self.label_text_input.input.set_text("")
        self._elements_panel.reset_controls()
        self._elements_panel.view_all_code_button.enable()
        self.closed_command()

class CreatePanel(ControlResizingPanel):
    manager: pygame_gui.UIManager
    elements_list: Elements
    elements_panel: ElementsPanel
    preview: Preview
    textures: Textures
    create_button_button: ControlButton
    create_background_button: ControlButton

    create_button_window: Optional[CreateButtonWindow]
    create_background_window: Optional[CreateBackgroundWindow]

    def __init__(
        self,
        elements: Elements,
        elements_panel: ElementsPanel,
        preview: Preview,
        textures: Textures,
        manager: pygame_gui.UIManager,
        parent_container: Optional[pygame_gui.core.IContainerLikeInterface] = None,
    ):
        super().__init__(
            PREVIEW_SIZE_X * PREVIEW_SCALE_X,
            0,
            pygame.display.get_window_size()[0] - (PREVIEW_SIZE_X * PREVIEW_SCALE_X),
            0,
            manager,
            parent_container
        )
        self.create_button_button = self.add_element_offset(
            lambda width, y, container: ControlButton(
                0,
                y,
                width,
                30,
                "Add Button",
                manager,
                container,
                self._open_create_button_window
            ),
            process_event=True
        )
        self.create_background_button = self.add_element_offset(
            lambda width, y, container: ControlButton(
                0,
                y,
                width,
                30,
                "Add Background",
                manager,
                container,
                self._open_create_background_window
            ),
            process_event=True
        )
        self.create_button_window = None
        self.create_background_window = None
        self.manager = manager
        self.elements_list = elements
        self.elements_panel = elements_panel
        self.preview = preview
        self.textures = textures

    def process_event(self, event: pygame.Event):
        super().process_event(event)
        if self.create_button_window != None:
            self.create_button_window.process_event(event)
        if self.create_background_window != None:
            self.create_background_window.process_event(event)

    def _open_create_button_window(self):
        if self.create_button_window == None:
            self.create_button_window = CreateButtonWindow(
                self.manager,
                self._close_create_button_window,
                self.elements_list,
                self.elements_panel,
                self.preview
            )
        self.create_button_window.show()
        self.create_button_button.disable()
        self.create_background_button.disable()

    def _close_create_button_window(self):
        if self.create_button_window != None:
            self.create_button_window.hide()
            self.create_button_window.disable()
            self.create_button_window.kill()
            self.create_button_window = None
        self.create_button_button.enable()
        self.create_background_button.enable()

    def _open_create_background_window(self):
        if self.create_background_window == None:
            self.create_background_window = CreateBackgroundWindow(
                self.manager,
                self._close_create_background_window,
                self.elements_list,
                self.elements_panel,
                self.textures,
                self.preview
            )
        self.create_button_button.disable()
        self.create_background_button.disable()

    def _close_create_background_window(self):
        if self.create_background_window != None:
            self.create_background_window.disable()
            self.create_background_window.hide()
            self.create_background_window.kill()
            self.create_background_window = None
        self.create_button_button.enable()
        self.create_background_button.enable()

    def disable(self):
        super().disable()
        if self.create_button_window != None:
            self.create_button_window.disable()
        if self.create_background_window != None:
            self.create_background_window.disable()

class PreviewManagementPanel(ControlResizingPanel):
    show_grid_checkbox: ControlCheckbox

    _preview: Preview

    def __init__(
        self,
        preview: Preview,
        manager: pygame_gui.UIManager,
        parent_container: Optional[pygame_gui.core.IContainerLikeInterface] = None
    ):
        checkbox_height = 30
        super().__init__(
            0, #PREVIEW_SIZE_X * PREVIEW_SCALE_X + checkbox_height,
            PREVIEW_SIZE_Y * PREVIEW_SCALE_Y, #pygame.display.get_window_size()[1] - checkbox_height - 6,
            pygame.display.get_window_size()[0] - (PREVIEW_SIZE_X * PREVIEW_SCALE_X),
            36,
            manager,
            parent_container
        )
        self.show_grid_checkbox = self.add_element_offset(
            lambda _, y, container: ControlCheckbox(
                0,
                y,
                checkbox_height,
                checkbox_height,
                "Show Grid",
                manager,
                container,
                self._show_preview_grid
            )
        )
        self._preview = preview

    def _show_preview_grid(self, state: bool):
        if state:
            self._preview.show_grid()
        else:
            self._preview.hide_grid()

class Control:
    manager: pygame_gui.UIManager
    panel: ControlPanel
    preview: Preview

    elements: Elements
    textures: Textures

    create_panel: CreatePanel
    elements_panel: ElementsPanel
    preview_management_panel: PreviewManagementPanel

    def __init__(self, manager: pygame_gui.UIManager, preview: Preview):
        self.manager = manager
        self.preview = preview
        self.elements = {}
        self.textures = Textures()
        self.elements_panel = ElementsPanel(
            65,
            self.elements,
            self.textures,
            self.preview,
            manager
        )
        self.create_panel = CreatePanel(
            self.elements,
            self.elements_panel,
            self.preview,
            self.textures,
            self.manager
        )
        self.preview_management_panel = PreviewManagementPanel(
            self.preview,
            manager
        )

    def update(self, time_delta: float):
        self.create_panel.update(time_delta)
        self.elements_panel.update(time_delta)
        self.preview_management_panel.update(time_delta)

    def process_event(self, event: pygame.Event):
        self.create_panel.process_event(event)
        self.elements_panel.process_event(event)
        self.preview_management_panel.process_event(event)
