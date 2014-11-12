#need to extend the interpreter for login using the extension
import pyio
#global definitions 
	#instance of the handler
handler_obj = None

def get(path):
	#route for the landing page
	if path is '/':
		f=open('./views/land.html')
		handler_obj.send_response(200)
		handler_obj.send_header('Content-type','text/html')
		handler_obj.end_headers()
		handler_obj.wfile.write(f.read())	
		return
	#this is for sending login values to the server
	
		
def getFile(name):
	loc=''
	if name.endswith(".js"):
		loc='./scripts/'+name
	if name.endswith(".css"):
		loc='./style/'+name
	if loc is '':	#this means that hasn't requested a file
		return False

	try:
		f=open(loc)	#need to handle IOError here for a file not present
	except IOError, e:
		print e.errno
		print e
		return
	handler_obj.send_response(200)
	handler_obj.send_header('Content-type','text/html')
	handler_obj.end_headers()
	handler_obj.wfile.write(f.read())
	return True

def post(path,args):
	#request for login
	if path=='/login':
		#send auth data using c sockets
		user = args['username']
		passwd = args['password']
		auth_str = '{"AUTH":"'+user+'$'+passwd+'"}'
		leng = len(auth_str)
		len_str = '{"LENGTH":'+leng.zfill(5)+'}'
		pyio.write(len_str)
		pyio.write(auth_str)
	return

def init(s):	
	global handler_obj
	handler_obj = s
