#include <libwebsockets.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <stdbool.h>



// For signaling a shutdown
static int interrupted = 0;


struct connection_list_node {
    struct lws *wsi;
    char *chat_room_id_buffer;
    unsigned char *u_id;
    struct connection_list_node *next;
};


struct connection_list_node *connection_list_head = NULL;


// The size of the buffer to receive and send messages
#define LWS_BUFFER_SIZE 4096

static int callback_chat(struct lws *wsi, enum lws_callback_reasons reason,
                         void *user, void *in, size_t len) {


    switch (reason) {
        case LWS_CALLBACK_ESTABLISHED:
	    struct connection_list_node *new_node = malloc(sizeof(struct connection_list_node));
            
           char userid_str[32];
            if (lws_get_urlarg_by_name(wsi, "userId", userid_str, sizeof(userid_str)) > 0) {
	
	      } 

	    new_node->chat_room_id_buffer=strdup((const char *)userid_str);	    
	   if (new_node) {
                new_node->wsi = wsi;
                new_node->next = connection_list_head;
		new_node->u_id = NULL;
                connection_list_head = new_node;
		}
        
            break;

        case LWS_CALLBACK_RECEIVE: {
    	  //  char chat_room_id_buffer[32];
	    unsigned char *userid_str;
	    bool u_id_message = false;
	    char *croom_id;
	    unsigned char *user_id;
	    bool being_used = false;
	    char *current_uid;
	    unsigned char *tempbuf;

	   struct connection_list_node *current = connection_list_head;

	   while(current){
		if (current->u_id == NULL && current->wsi == wsi){
			u_id_message = true;
			break;
			}
		current = current->next;
	   }
	   current = connection_list_head;

	   while (current) {
                if (current->wsi == wsi) {
			croom_id = current->chat_room_id_buffer;
			break;
		}
		current = current->next;
	   }
    
    	   user_id = (unsigned char*)malloc(len + 1);
    	   memcpy(user_id, in,len);
    	   user_id[len] = '\0';   // NULL-terminate
	   current = connection_list_head;
 
	   while (current) {
		   if (current->u_id != NULL && current->wsi != wsi) {
			if(strcmp(current->chat_room_id_buffer,croom_id) == 0 && strcmp(current->u_id,user_id) == 0 ) {

				const char *mess = "Name used";
				size_t message_len = strlen(mess);
				unsigned char buffer[LWS_PRE + message_len];
				memcpy(&buffer[LWS_PRE], mess, message_len);
				lws_write(wsi, &buffer[LWS_PRE], message_len, LWS_WRITE_TEXT);
				being_used = true;
				break;
			}
		   }
		   current = current->next;
	   }
	   
	   current = connection_list_head;
	   if(being_used == false) {
		   while (current) {
			 if(current->wsi == wsi) {
				 if(u_id_message == true){
					current->u_id = (unsigned char*)malloc(sizeof(user_id));
                   			strcpy(current->u_id,user_id);
					break;
				 }
			}
		    current=current->next;
		   }
	   }

      current = connection_list_head;
      unsigned char *strbuf;
      strbuf = (unsigned char*)malloc(sizeof(current->u_id));
      while (current) {
            if(current->wsi == wsi) {
		    if(current->u_id != NULL){
                       strcpy(strbuf,current->u_id);
		       strbuf[strlen(strbuf)] = '\0';
			break;
	    		}	
	    }
	    current = current->next;
      }
	  

	  if (u_id_message == false){ 

		   unsigned char buff[strlen(strbuf) + strlen(user_id) + 5 ] ;
		   strcpy(buff,"\n");
		   strcat(buff,strbuf);
                   strcat(buff,": ");
                   strcat(buff,user_id);
                   buff[strlen(buff)] = '\0';
                   char *buf2 = buff;
                   unsigned char *buf3 = (unsigned char *)malloc(LWS_PRE + sizeof(buff));
		   memcpy(&buf3[LWS_PRE],buf2 , sizeof(buff));

	           current = connection_list_head;

            while (current) {
                if (strcmp(current->chat_room_id_buffer,croom_id) == 0 ){
                   lws_write(current->wsi,&buf3[LWS_PRE],strlen(buff), LWS_WRITE_TEXT);
                       }
                current = current->next;
                }
	      free(buf3);
	  }
	  free(strbuf);
	  free(user_id);
          current = NULL;

          break;

        }

        case LWS_CALLBACK_CLOSED:
	{

            struct connection_list_node *current = connection_list_head;
            struct connection_list_node *prev = NULL;

            while (current) {
                if (current->wsi == wsi) {
                    if (prev) {
                        prev->next = current->next;
                    } else {
                        connection_list_head = current->next;
                    }
		    free(current->u_id);
                    free(current);
                    printf("Connection closed.\n");
                    break;
                }
                prev = current;
                current = current->next;
            }

            break;
	}

        break;
    }
    return 0;
}

// An array of supported protocols. An echo protocol is defined here.
static struct lws_protocols protocols[] = {
    {
        "echo-protocol",
        callback_chat,
        0, // Per-session data size
        LWS_BUFFER_SIZE, // RX buffer size
    },
    { NULL, NULL, 0, 0 } // Protocol list must end with a NULL entry
};

// Signal handler to gracefully stop the server
static void sigint_handler(int sig) {
    interrupted = 1;
}

int main(int argc, char **argv) {
    // Set up a signal handler for graceful termination
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sigint_handler;
    sigaction(SIGINT, &sa, NULL);

    struct lws_context *context;
    struct lws_context_creation_info info;

    // Initialize context creation info
    memset(&info, 0, sizeof(info));
    info.port = 8000; // Choose a port for the server
    info.protocols = protocols;
    info.gid = -1;
    info.uid = -1;
    info.options = 0;

    // Create the libwebsockets context
    context = lws_create_context(&info);
    if (!context) {
        printf("lws_create_context failed\n");
        return 1;
    }

    printf("libwebsockets chat server started on port %d...\n", info.port);

    // Main event loop
    while (!interrupted) {
        lws_service(context, 50); // Service the context every 50ms
    }
    // Destroy the context upon exiting the loop
    printf("Shutting down server...\n");
    lws_context_destroy(context);

    return 0;
}

