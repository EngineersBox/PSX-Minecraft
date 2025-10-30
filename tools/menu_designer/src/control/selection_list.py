import pygame_gui
from src.control.base import ControlBase

class ControlSelectionList(ControlBase):
    selection_list: pygame_gui.elements.UISelectionList

    def __init__(
        self,
        x: int,
        y: int,
        width: int,
        height: int,
        item_list: list[str | tuple[str, str]],
        manager: pygame_gui.UIManager,
        control_container: pygame_gui.core.IContainerLikeInterface
    ):
        super().__init__(x, y, width, height)
        self.selection_list = pygame_gui.elements.UISelectionList(
            relative_rect=self.rect,
            item_list=item_list,
            allow_multi_select=False,
            manager=manager,
            container=control_container
        )

    def disable(self):
        self.selection_list.disable()

    def enable(self):
        self.selection_list.enable()

    def hide(self):
        self.selection_list.hide()

    def show(self):
        self.selection_list.show()
