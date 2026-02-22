import os

os.environ["GDK_BACKEND"] = "x11"

import sys
import gi

import Instance

gi.require_version('Gtk', '4.0')
from gi.repository import Gtk, Gdk, GLib, GdkPixbuf

if __name__ == "__main__":
    app = Instance.Instance()
    app.run(sys.argv)
