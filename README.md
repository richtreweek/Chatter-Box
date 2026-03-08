# Chatter-Box
Web page chat box with server software;
This is Chatter Box,a web page chat box with server software.

js.html is a template web page with a java script user interface. Chatter box uses HTML Textareas so most attributes of the boxes can be        confiured with HTML.

You will need set the server URL and chatroom name in the page javascript. Both perameters are set in a clearly marked line in the page source.

Chat.c is the server code. Chatterbox server requires libwebsockets and glibc. Compile with "gcc chat.c -o server -lwebsockets".  
the server can handle infinite connections in infinite chatrooms.

The server has only been tested on ubuntu linux so i don't know if it will work with other linuxes. Let me know.

Enjoy!

Richard Treweek
rich.treweek@gmail.com

ps. Looking for work.
