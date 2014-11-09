#!/usr/bin/python
from BaseHTTPServer import BaseHTTPRequestHandler,HTTPServer
import urlparse,cgi,os
import pyio		#module for talking to the C server socket
#custom user made module
import router

class webHandler(BaseHTTPRequestHandler):
	
	def do_GET(self):
		router.init(self)
		path = urlparse.urlparse(self.path).path
		#this for fetching a file
		if router.getFile(path):	#if true means was requesting a file
			return
		#this is for fetching a view
		router.get(path)
		return
	
	def do_POST(self):
		length = int(self.headers['Content-Length'])
		post_data = urlparse.parse_qs(self.rfile.read(length).decode('utf-8'))
		path = urlparse.urlparse(self.path).path
		router.post(path,post_data)	#invoking the webapi and posting to the server
		return

def main():
	try:
    		server = HTTPServer(('localhost', 8080), webHandler)
		print pyio.write("someday",len("someday"))
    		print 'HTTPServer started'
		os.chdir('./../Webserver/')
    		server.serve_forever()
	except KeyboardInterrupt:
    		print 'server.socket.close()'
    		server.socket.close()
