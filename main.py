import os

os.environ["GDK_BACKEND"] = "x11"

import sys
import subprocess
import gi
import time

gi.require_version('Gtk', '4.0')
from gi.repository import Gtk, Gdk, GLib, GdkPixbuf

g_application_version = "a1.1"
g_code_name = "Inertia"


class Lumi(Gtk.ApplicationWindow):
    def __init__(self, app):
        super().__init__(application=app)

        self.set_title("Lumi")
        self.set_decorated(False)
        self.set_titlebar(None)

        css_provider = Gtk.CssProvider()
        css_provider.load_from_data(b"""
                window {
                    background-color: transparent;
                    box-shadow: none;
                }
                image {
                    background-color: transparent;
                    box-shadow: none;
                }
                decoration {
                    box-shadow: none;
                    background: none;
                }
                windowhandle {
                    background-color: transparent;
                    box-shadow: none;
                    border: none;
                }
            """)

        Gtk.StyleContext.add_provider_for_display(Gdk.Display.get_default(), css_provider, Gtk.STYLE_PROVIDER_PRIORITY_APPLICATION)

        self.texture_right = Gdk.Texture.new_from_filename("res/idle.png")
        self.texture_left = Gdk.Texture.new_from_filename("res/idle_var.png")
        self.texture_grabbed = Gdk.Texture.new_from_filename("res/grabbed.png")

        self.image = Gtk.Image.new_from_paintable(self.texture_right)
        self.image.set_pixel_size(300)

        self.handle = Gtk.WindowHandle()
        self.handle.set_child(self.image)
        self.set_child(self.handle)

        self.is_dragging = False

        click_controller = Gtk.GestureClick()
        click_controller.set_button(0)

        click_controller.set_propagation_phase(Gtk.PropagationPhase.CAPTURE)

        click_controller.connect("pressed", self.on_pressed)

        self.handle.add_controller(click_controller)

        motion_controller = Gtk.EventControllerMotion()

        motion_controller.connect("enter", self.on_status_check)
        motion_controller.connect("motion", self.on_status_check)

        self.handle.add_controller(motion_controller)

        self.pos_x = 100
        self.pos_y = 160
        self.walk_direction = 1

        self.present()

        GLib.timeout_add(1000, self.force_always_on_top)
        GLib.timeout_add(50, self.auto_walk)

    def auto_walk(self):
        if self.is_dragging:
            return True

        speed = 2

        self.pos_x += speed * self.walk_direction

        left_boundary = 200
        right_boundary = 500

        if self.pos_x > right_boundary:
            self.walk_direction = -1
            self.image.set_from_paintable(self.texture_left)
        elif self.pos_x < left_boundary:
            self.walk_direction = 1
            self.image.set_from_paintable(self.texture_right)

        try:
            cmd = f"wmctrl -r 'Lumi' -e 0,{self.pos_x},{self.pos_y},-1,-1"
            os.system(cmd)
        except:
            pass

        return True

    def force_always_on_top(self):
        try:
            print("[LUMI]: Trying to force pin...")
            os.system("wmctrl -r 'Lumi' -b add,above")
        except Exception as e:
            print(f"[LUMI]: Failed to pin window: {e}")
        return False

    def on_pressed(self, gesture, n_press, x, y):
        self.is_dragging = True
        self.image.set_from_paintable(self.texture_grabbed)

    def on_status_check(self, controller, x, y):
        event = controller.get_current_event()

        if event:
            state = event.get_modifier_state()

            if not (state & Gdk.ModifierType.BUTTON1_MASK):
                self.is_dragging = False

                if self.walk_direction == 1:
                    self.image.set_from_paintable(self.texture_right)
                else:
                    self.image.set_from_paintable(self.texture_left)

                self.sync_position()

    def sync_position(self):
        try:
            output = subprocess.check_output(["xdotool", "search", "--name", "Lumi", "getwindowgeometry", "--shell"], text=True)

            for i in output.splitlines():
                if i.startswith("X="):
                    self.pos_x = int(i.split("=")[1])
                elif i.startswith("Y="):
                    self.pos_y = int(i.split("=")[1])

            print(f"[LUMI]: Synced Position to: {self.pos_x} | {self.pos_y}")

        except Exception as e:
            print(f"[LUMI]: Couldn't Sync Position: {e}")


class LumiApplication(Gtk.Application):
    def __init__(self):
        super().__init__(application_id="me.nonenum.lumi")

    def do_activate(self):
        instance = Lumi(self)


if __name__ == "__main__":
    app = LumiApplication()
    app.run(sys.argv)
