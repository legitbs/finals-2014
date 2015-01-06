from __future__ import print_function
import tornado.httpserver
import tornado.web
import tornado.websocket
import tornado.ioloop
import tornado.gen

import tornadoredis

width = 1024
height = 768

# a list of table coordinates from top left to bottom right...
tables = [((width/9),(height/6)), ((width/9)*2,(height/6)), ((width/9)*3,(height/6)), 
          ((width/9)*4,(height/6)), ((width/9)*5,(height/6)), ((width/9)*6,(height/6)), 
          ((width/9)*7,(height/6)), ((width/9)*8,(height/6)), ((width/3),(height/6)*2), 
          ((width/3)*2,(height/6)*2), ((width/3),(height/6)*3), ((width/3)*2,(height/6)*3), 
          ((width/9),(height/6)*4), ((width/9)*2,(height/6)*4), ((width/9)*3,(height/6)*4), 
          ((width/9)*4,(height/6)*4), ((width/9)*5,(height/6)*4), ((width/9)*6,(height/6*4)), 
          ((width/9)*7,(height/6)*4), ((width/9)*8,(height/6)*4)]

c = tornadoredis.Client(host="10.3.1.5", port=6379)
c.connect()

class MainHandler(tornado.web.RequestHandler):
    def get(self):
        self.render("index.html", title="Team Flag Captures")
    
    def check_origin(self, origin):
        return True

class MessageHandler(tornado.websocket.WebSocketHandler):
    def __init__(self, *args, **kwargs):
        super(MessageHandler, self).__init__(*args, **kwargs)
        self.listen()

    @tornado.gen.engine
    def listen(self):
        self.client = tornadoredis.Client()
        self.client.connect()
        yield tornado.gen.Task(self.client.subscribe, 'scorebot_production')
        self.client.listen(self.on_message)

    def on_message(self, msg):
        print(msg)
        if msg.kind == 'message':
            self.write_message(str(msg.body))
        if msg.kind == 'disconnect':
            # Do not try to reconnect, just send a message back
            # to the client and close the client connection
            self.write_message('The connection terminated '
                               'due to a Redis server error.')
            self.close()

    def on_close(self):
        if self.client.subscribed:
            self.client.unsubscribe('scorebot_production')
            self.client.disconnect()

    def check_origin(self, origin):
        return True


application = tornado.web.Application([
    (r'/', MessageHandler),
    (r'/nothing', MainHandler)
])

if __name__ == '__main__':
    http_server = tornado.httpserver.HTTPServer(application)
    http_server.listen(8888)
    print('tornado-redis running at 0.0.0.0:8888\nQuit with CONTROL-C')
    tornado.ioloop.IOLoop.instance().start()
