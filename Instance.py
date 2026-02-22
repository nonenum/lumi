import os

os.environ["GDK_BACKEND"] = "x11"

import sys
import gi

import Lumi

gi.require_version('Gtk', '4.0')
from gi.repository import Gtk


class Instance(Gtk.Application):
    def __init__(self):
        super().__init__(application_id="me.nonenum.lumi")

    def do_activate(self):
        instance = Lumi.Lumi(self)
