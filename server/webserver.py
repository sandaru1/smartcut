#!/usr/bin/env python

import string,cgi,time
from os import curdir, sep
from BaseHTTPServer import BaseHTTPRequestHandler, HTTPServer
import MediaLibrary

class ServerHandler(BaseHTTPRequestHandler): 

    def do_GET(self): 
        temp = self.path.split('?')
        path = temp[0]
        params = {}
        if len(temp) > 1 :
            params = cgi.parse_qs(temp[1])
        try: 
            if path.endswith("Echo"): 
                self.send_response(200) 
                self.send_header('Content-type','text/html') 
                self.end_headers() 
                self.wfile.write(params['input']) 
                return 
            if path.endswith("MediaLibrary/GetContent"):
                self.send_response(200) 
                self.send_header('Content-type','text/html') 
                self.end_headers() 
                self.wfile.write(MediaLibrary.getContent()) 
                return 
            if path.endswith("MediaLibrary/UploadClip"):
                self.send_response(200) 
                self.send_header('Content-type','text/html') 
                self.end_headers() 
                MediaLibrary.uploadClip(params['file'][0])
                return 
            if path.endswith("MediaLibrary/RemoveClip"):
                self.send_response(200) 
                self.send_header('Content-type','text/html') 
                self.end_headers() 
                MediaLibrary.removeClip(params['name'][0])
                return 
            if path.endswith("Effects/AutoSplit"):
                self.send_response(200) 
                self.send_header('Content-type','text/html') 
                self.end_headers() 
                MediaLibrary.AutoSplit(params['name'][0])
                return 
                
        except IOError: 
            self.send_error(404,'File Not Found') 

def main(): 
    try: 
        server = HTTPServer(('', 8080), ServerHandler) 
        print 'started httpserver...' 
        server.serve_forever() 
    except KeyboardInterrupt: 
        server.socket.close() 

if __name__ == '__main__': 
    main() 
