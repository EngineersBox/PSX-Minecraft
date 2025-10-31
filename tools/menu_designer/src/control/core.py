from os import close
from pathlib import Path
from typing import Callable, Optional, cast
import pygame, pygame_gui
from src.codegen.gen import FONTS, gen_html_code
from src.control.button import ControlButton
from src.control.checkbox import ControlCheckbox
from src.control.drop_down import ControlDropDown
from src.control.file_dialog import ControlFileDialog
from src.control.horizontal_slider import ControlHorizontalSlider
from src.control.label import ControlLabel
from src.control.panel import ControlPanel
from src.control.resizing_panel import ControlResizingPanel
from src.control.selection_list import ControlSelectionList
from src.control.text_box import ControlTextBox
from src.control.text_input import ControlTextInput, number_only_validator
from src.control.window import ControlWindow, ControlWindowClosedCommand
from src.preview.core import Preview
from src.preview.const import PREVIEW_SIZE_X, PREVIEW_SIZE_Y, PREVIEW_SCALE_X, PREVIEW_SCALE_Y
from src.preview.background import PreviewBackground
from src.preview.button import PreviewButton
from src.preview.element import PreviewElement

# TODO : Refactor to support generic PreviewElement management
#        and created tabbed layout for buttons and background
#        elements

type Elements = dict[str, tuple[str, str]]

class ButtonModifiersPanel(ControlResizingPanel):
    diasbled_checkbox: ControlCheckbox

    _get_selected_element: Callable[[], Optional[PreviewElement]]

    def __init__(
        self,
        y: int,
        manager: pygame_gui.UIManager,
        parent_container: pygame_gui.core.IContainerLikeInterface,
        get_selected_element: Callable[[], Optional[PreviewElement]]
    ):
        super().__init__(
            PREVIEW_SIZE_X * PREVIEW_SCALE_X,
            y,
            pygame.display.get_window_size()[0] - (PREVIEW_SIZE_X * PREVIEW_SCALE_X),
            0,
            manager,
            parent_container
        )
        self.diasbled_checkbox= self.add_element_offset(
            lambda _, y, container: ControlCheckbox(
                0,
                y,
                30,
                30,
                "Disabled",
                manager,
                container,
                self._set_button_disabled_state
            )
        )
        self._get_selected_element = get_selected_element

    def _set_button_disabled_state(self, disabled: bool):
        selected = self._get_selected_element()
        if selected == None or type(selected) != PreviewButton:
            return
        selected.set_disabled(disabled)

    def reset_controls(self):
        self.diasbled_checkbox.disable()

class BackgroundModifiersPanel(ControlPanel):

    def __init__(self):
        pass

    def reset_controls(self):
        pass

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
        self.code_close = ControlButton(
            pygame.display.get_window_size()[0] - 106,
            pygame.display.get_window_size()[1] - 36,
            100,
            30,
            "Close",
            manager,
            self.panel,
            close_command
        )
        self.code_font_drop_down = self.add_element(
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

    pos_x_label: ControlLabel
    pos_y_label: ControlLabel
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
        manager: pygame_gui.UIManager,
        parent_container: pygame_gui.core.IContainerLikeInterface,
        elements: Elements,
        preview: Preview
    ):
        super().__init__(
            0,
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
            )
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
            )
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
            )
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
            )
        )
        self.pos_x_label = self.add_element_offset(
            lambda width, y, container: ControlLabel(
                0,
                y,
                width,
                30,
                "X: N/A",
                manager,
                container
            )
        )
        self.pos_y_label = self.add_element_offset(
            lambda width, y, container: ControlLabel(
                0,
                y,
                width,
                30,
                "Y: N/A",
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
            )
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
            )
        )
        self.background_modifiers_panel = BackgroundModifiersPanel()
        self.button_modifiers_panel = ButtonModifiersPanel(
            self.y_offset + 1,
            manager,
            parent_container,
            self.get_selected_element
        )
        self.code_panel = None
        self._manager = manager
        self._elements = elements
        self._preview = preview

    def process_event(self, event: pygame.Event):
        super().process_event(event)
        self.button_modifiers_panel.process_event(event)
        self.background_modifiers_panel.process_event(event)
        if self.code_panel is not None:
            self.code_panel.process_event(event)

    def reset_controls(self):
        self.remove_element_button.disable()
        self.view_element_code_button.disable()
        if len(self.elements) == 0:
            self.view_all_code_button.disable()
        self.pos_x_label.label.set_text("X: N/A")
        self.pos_y_label.label.set_text("Y: N/A")
        self.width_label.label.set_text("Width: N/A")
        self.width_slider.set_value(-1)
        self.height_label.label.set_text("Height: N/A")
        self.height_slider.set_value(-1)
        self.button_modifiers_panel.reset_controls()
        self.background_modifiers_panel.reset_controls()

    def get_selected_element(self) -> Optional[PreviewElement]:
        selected = self.elements_list.selection_list.get_single_selection(include_object_id=True)
        if selected == None:
            return None
        return self._preview.get_element(selected[1])

    def _selected_element(self, id: tuple[str, str]):
        self.enable()
        self.show()
        selected = self._preview.get_element(id[1])
        if selected is not None:
            width = selected.get_width()
            self.width_slider.set_value(width)
            self.width_label.label.set_text(f"Width: {width}")

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

    def _width_slider_moved(self, width: int):
        self.height_label.set_text(f"Width: {width}")
        selected = self.get_selected_element()
        if selected == None:
            return
        selected.set_width(width)

    def _height_slider_moved(self, height: int):
        self.width_label.set_text(f"Height: {height}")
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
        self.width_input = ControlTextInput(
            (window_width // 2) - 50,
            65,
            100,
            30,
            manager,
            self.window,
            number_only_validator
        )
        self.height_input = ControlTextInput(
            (window_width // 2) - 50,
            100,
            100,
            30,
            manager,
            self.window,
            number_only_validator
        )
        button_width = 70
        self.create_button = ControlButton(
            (window_width // 2) - (button_width // 2),
            135,
            button_width,
            30,
            "Create",
            manager,
            self.window,
            self._add_button
        )
        self.create_button.disable()
        self._elements = elements
        self._elements_panel = elements_panel
        self._preview = preview

    def process_event(self, event: pygame.Event):
        self.width_input.process_event(event)
        self.height_input.process_event(event)
        if (event.type == pygame_gui.UI_TEXT_ENTRY_CHANGED
            and event.ui_element == self.label_text_input):
            if len(self.label_text_input.input.get_text()) > 0:
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
        self.label_text_input.input.set_text("")
        self._elements_panel.reset_controls()
        self._elements_panel.view_all_code_button.enable()

    def show(self):
        super().show()
        self.label_text_input.input.clear()

class CreateBackgroundWindow(ControlWindow):
    image_file_dialog: ControlFileDialog
    label_text_input: ControlTextInput
    width_label: ControlLabel
    width_input: ControlTextInput
    height_label: ControlLabel
    height_input: ControlTextInput
    create_button: ControlButton

    _image: Optional[pygame.Surface]
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
        self.image_file_dialog = ControlFileDialog(
            0,
            0,
            PREVIEW_SIZE_X,
            PREVIEW_SIZE_Y,
            "Select Background Image",
            self._bg_image_selected,
            self._close_bg_image_file_dialog,
            manager
        )
        self.image_file_dialog.disable()
        self.image_file_dialog.hide()
        self.label_text_input = ControlTextInput(
            (window_width // 2) - 50,
            30,
            100,
            30,
            manager,
            self.window,
            placeholder_text="Name..."
        )
        self.width_label = ControlLabel(
            (window_width // 2) - 50 - 30,
            60,
            30,
            30,
            "Width:",
            manager,
            self.window,
        )
        self.width_input = ControlTextInput(
            (window_width // 2) - 50,
            60,
            100,
            30,
            manager,
            self.window,
            number_only_validator,
            initial_text="100"
        )
        self.width_label = ControlLabel(
            (window_width // 2) - 50 - 30,
            60,
            100,
            30,
            "Height:",
            manager,
            self.window,
        )
        self.height_input = ControlTextInput(
            (window_width // 2) - 50,
            65,
            100,
            30,
            manager,
            self.window,
            number_only_validator,
            initial_text="100"
        )
        self.create_button = ControlButton(
            (window_width // 2) - 35,
            110,
            70,
            30,
            "Create",
            manager,
            self.window,
            self._add_background
        )
        self._elements = elements
        self._elements_panel = elements_panel
        self._preview = preview

    def _bg_image_selected(self, path: Path):
        try:
            self._image = pygame.image.load(path)
        except Exception as e:
            print(f"Invalid image: {e}")
            self._image = None

    def _close_bg_image_file_dialog(self):
        self.image_file_dialog.disable()
        self.image_file_dialog.hide()

    def _add_background(self):
        name = self.label_text_input.get_text()
        if (len(name) == 0 or self._image == None):
            return
        background_id = self._preview.add_background(self._image)
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

class CreatePanel(ControlResizingPanel):
    create_button_button: ControlButton
    create_background_button: ControlButton

    create_button_window: CreateButtonWindow
    create_background_window: CreateBackgroundWindow

    def __init__(
        self,
        elements: Elements,
        elements_panel: ElementsPanel,
        preview: Preview,
        manager: pygame_gui.UIManager,
        parent_container: pygame_gui.core.IContainerLikeInterface,
    ):
        super().__init__(
            PREVIEW_SIZE_X,
            0,
            pygame.display.get_window_size()[1] - PREVIEW_SIZE_X,
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
            )
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
            )
        )
        self.create_button_window = CreateButtonWindow(
            manager,
            self._close_create_button_window,
            elements,
            elements_panel,
            preview
        )
        self.create_background_window = CreateBackgroundWindow(
            manager,
            self._close_create_background_window,
            elements,
            elements_panel,
            preview
        )

    def _open_create_button_window(self):
        self.create_button_window.show()
        self.create_button_button.disable()
        self.create_background_button.disable()

    def _close_create_button_window(self):
        self.create_button_window.hide()

    def _open_create_background_window(self):
        self.create_background_window.show()
        self.create_button_button.disable()
        self.create_background_button.disable()

    def _close_create_background_window(self):
        self.create_background_window.hide()

    def disable(self):
        super().disable()
        self.create_button_window.disable()
        self.create_background_window.disable()

class PreviewManagementPanel(ControlPanel):
    show_grid_checkbox: ControlCheckbox
    open_bg_image_file_dialog: ControlButton
    bg_image_file_dialog: ControlFileDialog

    def __init__(self):
        pass

class Control:
    manager: pygame_gui.UIManager
    panel: ControlPanel
    preview: Preview

    elements: Elements

    create_panel: ControlResizingPanel

    elements_panel: ControlResizingPanel
    element_list: ControlSelectionList

    modify_button_panel: ControlResizingPanel
    modify_background_panel: ControlResizingPanel

    button_text_input: ControlTextInput
    pos_label: ControlLabel
    width_label: ControlLabel
    width_slider: ControlHorizontalSlider
    button_disabled_checkbox: ControlCheckbox
    remove_button: ControlButton
    view_button_code: ControlButton
    view_all_button_code: ControlButton

    preview_management_panel: ControlResizingPanel
    show_grid_checkbox: ControlCheckbox
    button_open_bg_image_file_dialog: ControlButton
    bg_image_file_dialog: ControlFileDialog

    code_panel: Optional[ControlPanel]
    code_view: ControlTextBox
    code_close: ControlButton
    code_font_drop_down: ControlDropDown

    def __init__(self, manager: pygame_gui.UIManager, preview: Preview):
        self.manager = manager
        self.preview = preview
        self.elements = {}
        self.elements_panel_elements()
        self.create_panel_elements()
        self.modify_button_panel_elements()
        self.preview_management_panel_elements()
        self.code_panel = None

    def create_panel_elements(self):
        self.create_panel = ControlResizingPanel(
            self.preview.rect.x + self.preview.rect.width,
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

    def modify_button_panel_elements(self):
        self.modify_button_panel = ControlResizingPanel(
            self.preview.rect.x + self.preview.rect.width,
            int(self.create_panel.y_offset + 20),
            pygame.display.get_window_size()[0] - self.preview.rect.width,
            0,
            self.manager
        )
        self.modify_button_panel.add_element_offset(
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
        self.element_list = self.modify_button_panel.add_element_offset(
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
        self.pos_label = self.modify_button_panel.add_element_offset(
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
        self.width_label = self.modify_button_panel.add_element_offset(
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
        self.width_slider = self.modify_button_panel.add_element_offset(
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
        self.button_disabled_checkbox = self.modify_button_panel.add_element_offset(
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
        self.remove_button = self.modify_button_panel.add_element_offset(
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
        self.view_button_code = self.modify_button_panel.add_element_offset(
            lambda width, y, container: ControlButton(
                0,
                y,
                width,
                30,
                "View Button Code",
                self.manager,
                container,
                self._open_selected_element_code_panel
            )
        )
        self.view_button_code.disable()
        self.view_all_button_code = self.modify_button_panel.add_element_offset(
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

    def preview_management_panel_elements(self):
        self.preview_management_panel = ControlResizingPanel(
            self.preview.rect.x + self.preview.rect.width + 1,
            int(self.modify_button_panel.rect.y + self.modify_button_panel.y_offset + 20),
            pygame.display.get_window_size()[0] - self.preview.rect.width,
            0,
            self.manager
        )
        self.show_grid_checkbox = self.preview_management_panel.add_element_offset(
            lambda _, y, container: ControlCheckbox(
                0,
                y,
                20,
                20,
                "Show Grid",
                self.manager,
                container,
                self._show_grid
            ),
            process_event=True
        )
        self.button_open_bg_image_file_dialog = self.preview_management_panel.add_element_offset(
            lambda width, y, container: ControlButton(
                0,
                y,
                width,
                30,
                "Select Background Image",
                self.manager,
                container,
                self._open_bg_image_file_dialog
            ),
            process_event=True
        )
        self.bg_image_file_dialog = ControlFileDialog(
            0,
            0,
            int(self.preview.container.get_abs_rect().width),
            int(self.preview.container.get_abs_rect().height),
            "Select Background Image",
            self._bg_image_selected,
            self._close_bg_image_file_dialog,
            self.manager
        )

    def update(self, time_delta: float):
        self.create_panel.update(time_delta)
        self.modify_button_panel.update(time_delta)
        self.preview_management_panel.update(time_delta)
        selected = self._get_selected_element()
        if selected is not None:
            pos = selected.get_pos()
            self.pos_label.label.set_text(f"X: {pos[0]} Y: {pos[1]}")
        if self.code_panel is not None:
            self.code_panel.update(time_delta)

    def process_event(self, event: pygame.Event):
        self.create_panel.process_event(event)
        self.modify_button_panel.process_event(event)
        self.preview_management_panel.process_event(event)
        self.bg_image_file_dialog.process_event(event)
        if (event.type == pygame_gui.UI_SELECTION_LIST_NEW_SELECTION
            and event.ui_element == self.element_list.selection_list):
            self.width_slider.enable()
            self.remove_button.enable()
            self.button_disabled_checkbox.enable()
            self.view_button_code.enable()
            selected = self._get_selected_element()
            if selected is not None:
                width = selected.get_width()
                self.width_slider.set_value(width)
                self.width_label.label.set_text(f"Width: {width}")
        if (event.type == pygame_gui.UI_SELECTION_LIST_DROPPED_SELECTION
            and event.ui_element == self.element_list.selection_list):
            self._reset_selection_controls()
        if (event.type == pygame_gui.UI_HORIZONTAL_SLIDER_MOVED
            and event.ui_element == self.width_slider.slider):
            selected = self._get_selected_element()
            if selected is not None:
                width = self.width_slider.get_value()
                selected.set_width(width)
                self.width_label.label.set_text(f"Width: {width}")
        if self.code_panel is not None:
            self.code_panel.process_event(event)

    def _add_button(self):
        text = self.button_text_input.input.get_text()
        if len(text) == 0:
            return
        button_id = self.preview.add_button(text)
        self.elements[button_id] = (text, button_id)
        self.element_list.selection_list.set_item_list(list(self.elements.values()))
        self.button_text_input.input.set_text("")
        self._reset_selection_controls()
        self.view_all_button_code.enable()

    def _remove_button(self):
        selected = self.element_list.selection_list.get_single_selection(include_object_id=True)
        if selected == None:
            return
        self.elements.pop(selected[1])
        self.preview.remove_element(selected[1])
        self.element_list.selection_list.set_item_list(list(self.elements.values()))
        self._reset_selection_controls()

    def _get_selected_element(self) -> Optional[PreviewElement]:
        selected = self.element_list.selection_list.get_single_selection(include_object_id=True)
        if selected == None:
            return None
        return self.preview.get_element(selected[1])

    def _set_button_disabled(self, disabled: bool):
        selected = self._get_selected_element()
        if selected == None or type(selected) != PreviewButton:
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
        if len(self.elements) == 0:
            self.view_all_button_code.disable()

    def _close_code_panel(self):
        self.code_view.kill()
        self.code_close.kill()
        if self.code_panel is not None:
            self.code_panel.kill()
        self.preview.show()
        self.create_panel.show()
        self.modify_button_panel.show()

    def _open_code_panel(self):
        self.preview.hide()
        self.create_panel.hide()
        self.modify_button_panel.hide()
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
        self.code_font_drop_down = self.code_panel.add_element(
            ControlDropDown(
                0,
                pygame.display.get_window_size()[1] - 36,
                100,
                30,
                # TODO: Use constant of fonts list
                ["monocraft"],
                self._update_code_view_font,
                self.manager,
                self.code_panel.panel
            ),
            process_event=True
        )

    def _open_selected_element_code_panel(self):
        selected = self._get_selected_element()
        if selected == None:
            return
        self._open_code_panel()
        self.code_view.set_text(gen_html_code(selected))

    def _open_all_button_code_panel(self):
        code_chunks = []
        for element_id in self.elements.keys():
            element = self.preview.get_element(element_id)
            if element == None:
                continue
            code_chunks.append(gen_html_code(element))
        self._open_code_panel()
        self.code_view.set_text("\n".join(code_chunks))

    def _update_code_view_font(self, font_selection: tuple[str, str]):
        # TODO: Complete this
        pass

    def _show_grid(self, show: bool):
        if show:
            self.preview.show_grid()
        else:
            self.preview.hide_grid()

    def _open_bg_image_file_dialog(self):
        self.button_open_bg_image_file_dialog.disable()
        self.bg_image_file_dialog.open()

    def _close_bg_image_file_dialog(self):
        self.button_open_bg_image_file_dialog.enable()

    def _bg_image_selected(self, path: Path):
        self._close_bg_image_file_dialog()
        try:
            image = pygame.image.load(path)
            self.preview.set_background_image(image)
        except Exception as e:
            # TODO: Show dialog with this error
            print(e)
            
