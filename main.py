from gi.repository import Gtk, Gdk, GLib
import sys
import gi

gi.require_version('Gtk', '4.0')


g_application_version = "a1.0"
g_code_name = "Unshattered"


class Lumi(Gtk.ApplicationWindow):
    def __init__(self, app):
        super().__init__(application=app)

        self.set_title("Lumi")
        self.set_decorated(False)

        css_provider = Gtk.CssProvider()
        css_provider.load_from_data(b"""
                window { background-color: transparent; }
                image { background-color: transparent; }
                windowhandle { background-color: transparent; }
            """)

        Gtk.StyleContext.add_provider_for_display(Gdk.Display.get_default(
        ), css_provider, Gtk.STYLE_PROVIDER_PRIORITY_APPLICATION)

        self.image = Gtk.Image.new_from_file("res/idle.png")
        self.image.set_pixel_size(300)

        self.handle = Gtk.WindowHandle()
        self.handle.set_child(self.image)
        self.set_child(self.handle)

        click_controller = Gtk.GestureClick()
        click_controller.set_button(0)

        click_controller.set_propagation_phase(Gtk.PropagationPhase.CAPTURE)

        click_controller.connect("pressed", self.on_pressed)

        self.handle.add_controller(click_controller)

        motion_controller = Gtk.EventControllerMotion()

        motion_controller.connect("enter", self.on_status_check)
        motion_controller.connect("motion", self.on_status_check)

        self.handle.add_controller(motion_controller)

        self.present()

    def on_pressed(self, gesture, n_press, x, y):
        self.image.set_from_file("res/grabbed.png")

    def on_status_check(self, controller, x, y):
        event = controller.get_current_event()

        if event is None:
            return

        state = event.get_modifier_state()

        if not (state & Gdk.ModifierType.BUTTON1_MASK):
            self.image.set_from_file("res/idle.png")


class LumiApplication(Gtk.Application):
    def __init__(self):
        super().__init__(application_id="me.nonenum.lumi")

    def do_activate(self):
        instance = Lumi(self)


if __name__ == "__main__":
    app = LumiApplication()
    app.run(sys.argv)
