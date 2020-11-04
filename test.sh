#!/bin/sh

DOCUMENT_ROOT="/tmp" \
GATEWAY_INTERFACE="CGI/1.1" \
HTTP_ACCEPT="text/html" \
HTTP_HOST="localhost" \
HTTP_USER_AGENT="gnegne/5.0" \
PATH_INFO="/games/godot" \
QUERY_STRING="" \
REMOTE_ADDR="127.0.0.1" \
REMOTE_PORT="63555" \
REQUEST_METHOD="GET" \
REQUEST_URI="/cgi-bin/webpkg.cgi/games/godot" \
SCRIPT_NAME="/cgi-bin/webpkg.cgi" \
SERVER_ADDR="127.0.0.1" \
SERVER_PORT="80" \
SERVER_PROTOCOL="HTTP/1.1" \
./webpkg
