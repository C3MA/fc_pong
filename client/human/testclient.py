#!/usr/bin/env python

import socket
import gtk

class Sender(gtk.Window):
	def __init__(self, server):
		super(Sender, self).__init__()

		self.set_title("PongClient")
		self.set_size_request(100, 100)
		self.set_resizable(False)
		self.set_position(gtk.WIN_POS_CENTER)

		self.connect("key-press-event", self.keyPress)
		self.connect("destroy", gtk.main_quit)

		self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		self.sock.connect(server)

		self.show_all()

	def keyPress(self, widget, event):
		key = event.keyval
		if key == 115:
			self.sock.sendall("+")
		elif key == 119:
			self.sock.sendall("-")


if __name__ == "__main__":
	s = Sender(("localhost", 34333))
	gtk.main()
