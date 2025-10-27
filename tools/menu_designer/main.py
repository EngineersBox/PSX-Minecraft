from __future__ import absolute_import
import pygame, pygame_gui
from src.menu_designer import MenuDesigner

def main():
    pygame.init()
    pygame.display.set_caption("Menu Designer")
    window_surface = pygame.display.set_mode(size=(1200,720))

    background = pygame.Surface((1280, 720))
    background.fill(pygame.Color("#232323"))

    gui_manager = pygame_gui.UIManager((1280, 720))
    designer = MenuDesigner(gui_manager)

    pygame.mouse.set_relative_mode(True)
    pygame.mouse.set_visible(True)

    is_running = True

    clock = pygame.time.Clock()
    while is_running:
        time_delta = clock.tick(60) / 1000.0
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                is_running = False
            gui_manager.process_events(event)
            designer.process_event(event)
        gui_manager.update(time_delta)
        designer.update(time_delta)
        window_surface.blit(background, (0, 0))
        gui_manager.draw_ui(window_surface)
        pygame.display.update()

if __name__ == "__main__":
    main()
