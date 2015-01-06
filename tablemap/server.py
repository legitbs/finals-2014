import os, sys
import tornado.httpserver
import tornado.ioloop
import tornado.web
import tornado.websocket
import json
import random
import time
import redis
import ast
import traceback
import tornadoredis
import thread

# viz overlay functions to trigger OSC events
from overlay import *

width = 1152
height = 864

clients = []

# a list of table coordinates from top left to bottom right...
tables = [((width/9),(height/6)), ((width/9)*2,(height/6)), ((width/9)*3,(height/6)), 
          ((width/9)*4,(height/6)), ((width/9)*5,(height/6)), ((width/9)*6,(height/6)), 
          ((width/9)*7,(height/6)), ((width/9)*8,(height/6)), ((width/3),(height/6)*2), 
          ((width/3)*2,(height/6)*2), ((width/3),(height/6)*3), ((width/3)*2,(height/6)*3), 
          ((width/9),(height/6)*4), ((width/9)*2,(height/6)*4), ((width/9)*3,(height/6)*4), 
          ((width/9)*4,(height/6)*4), ((width/9)*5,(height/6)*4), ((width/9)*6,(height/6*4)), 
          ((width/9)*7,(height/6)*4), ((width/9)*8,(height/6)*4)]

teamlist = [{"teamname":"Plaid Parliament of Pwning","x":0, "y":5,
             "shorthand": "PPP", "type": "table", "loc":10},
            {"teamname":"9447", "x": 0, "y": 5,
             "shorthand": "9447", "type": "table", "loc":3},
            {"teamname": "Reckless Abandon", "x":0, "y":5,
             "shorthand": "Reckless Abandon", "type": "table", "loc":6},
            {"teamname":"Routards", "x": 0, "y": 5,
             "shorthand": "Routards", "type": "table", "loc":15},
            {"teamname":"raon_ASRT", "x": 0, "y": 5,
             "shorthand": "raon_ASRT", "type":"table", "loc":9},
            {"teamname":"KAIST GoN", "x":0, "y":5,
             "shorthand": "KAIST GoN", "type":"table", "loc":14},
            {"teamname": "shellphish", "x":0, "y":5,
             "shorthand": "shellphish", "type":"table", "loc":19},
            {"teamname": "CodeRed", "x":0, "y":5,
             "shorthand": "CodeRed", "type":"table", "loc":16},
            {"teamname": "HITCON", "x":0, "y":5,
             "shorthand": "HITCON", "type":"table", "loc":7},
            {"teamname": "blue-lotus", "x":0, "y":100,
             "shorthand": "blue-lotus", "type": "table", "loc":5},
            {"teamname": "HackingForChiMac", "x":0, "y":100,
             "shorthand": "Hacking For Chi Mac", "type":"table", "loc":13},
            {"teamname": "(Mostly) Men in Black Hats", "x":0, "y":200,
             "shorthand": "(Mostly) Men in Black Hats", "type":"table", "loc":8},  
            {"teamname": "w3stormz", "x":0, "y":200,
             "shorthand": "w3stormz", "type":"table", "loc":12},
            {"teamname": "More Smoked Leet Chicken", "x":0, "y":300,
             "shorthand": "More Smoked Leet Chicken", "type":"table", "loc":11},
            {"teamname": "Dragon Sector", "x":0, "y":300, 
             "shorthand": "Dragon Sector", "type":"table", "loc":18},
            {"teamname": "[SEWorks]penthackon", "x":0, "y":300,
             "shorthand": "[SEWorks] penthackon", "type":"table", "loc":4},
            {"teamname": "StratumAuhuur", "x":0, "y":300, 
             "shorthand": "Stratum Auhuur", "type":"table", "loc":1},
            {"teamname": "Gallopsled", "x":0, "y":300,
             "shorthand": "Gallopsled", "type":"table", "loc":17},       
            {"teamname": "BalalaikaCr3w", "x":0, "y": 300,
             "shorthand": "Balalaika Cr3w", "type":"table", "loc":0},
            {"teamname": "Binja", "x":0, "y":300,
             "shorthand": "Binja", "type":"table", "loc":2}] 

servrgb = {"eliza":"#957bb7",
             "badger":"#fd0e67",
             "wdub":"#fe5b35",
             "justify":"#fe7518",
             "imap":"#91ccd6"}  

#r = redis.Redis("10.3.1.5")
#pub = r.pubsub()
#pub.subscribe("scorebot_production")

class IndexHandler(tornado.web.RequestHandler):
  @tornado.web.asynchronous
  def get(request):
    request.render("index.html")

class WebSocketChatHandler(tornado.websocket.WebSocketHandler):
  def __init__(self, *args, **kwargs):
    super(WebSocketChatHandler, self).__init__(*args, **kwargs)
    self.listen()

  @tornado.gen.engine
  def listen(self):
    #clients.append(self)

    print "open", "WebSocketChatHandler"

    #clients.append(self)

    self.sent_teams = 0

    try:
       self.client = tornadoredis.Client("10.3.1.5")
       self.client.connect()
       yield tornado.gen.Task(self.client.subscribe, "scorebot_production")
       self.client.listen(self.on_message)
    except Exception as ex:
       print ex

  def on_message(self, msg):
     print msg
     if self.sent_teams == 0:
         print "init'ing teams"
         teams = self.init_teams()
         map(self.send_team, teams)
         self.sent_teams = 1

     self.write_message(json.dumps(self.gen_pwn(msg)))

  def send_team(self, team):
    a = json.dumps(team)
    self.write_message(json.dumps(team))

  def on_close(self):
    print "closing"

  def check_origin(self, origin):
    return True

  def gen_pwn(self,indata):
    import pprint
    #indata = flag
    #pprint.pprint(indata)
    if len(indata) == 0:
        return None

    try:
       data = json.loads(indata.body)
    except Exception as ex:
       print "first exception",ex
       return None

    #pprint.pprint(data)

    try:
        print data["event_name"]
        if data["event_name"] == "redemption":
            print "got redemption"
            from_team = data["event_body"]["redeeming_team"]["name"]
            to_team = data["event_body"]["owned_team"]["name"]
            service = data["event_body"]["service"]["name"]

            #check if its first blood to send a trigger out to viz
            if data["event_body"]["redemption_stats"]["service"] == 1:
                print "first blood!"
                thread.start_new_thread(post_firstblood(from_team,service))

            print from_team, to_team, service
            return dict({"type": "pwn", "from": from_team, "to": to_team, "service": service, "servicergb": servrgb[service] })

        elif data["event_name"] == "round_finish":
            print "round finsihed"
            if data["event_body"]["places_changed"] == True:
                print "rank change!"
                thread.start_new_thread(rank_change())

        return None
    except Exception as e:
        print("in pwn func:",e)
        return None

  def init_teams(self):
    for t in (teamlist):
      t["x"] = tables[t["loc"]][0]
      t["y"] = tables[t["loc"]][1]
    return teamlist

app = tornado.web.Application([(r'/websocket', WebSocketChatHandler), (r'/', IndexHandler)])
app.listen(8888)
#http_server = tornado.httpserver.HTTPServer(app)
#http_server.listen(8888)
print "waiting on connections"
tornado.ioloop.IOLoop.instance().start()
