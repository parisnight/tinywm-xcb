/* print display information and query the tree for children */

#include <stdio.h>
#include <stdlib.h> /* for free() */
#include <xcb/xcb.h>
#include <inttypes.h>

       void my_example(xcb_connection_t *conn, xcb_window_t window) {
           xcb_query_tree_cookie_t cookie;
           xcb_query_tree_reply_t *reply;

           cookie = xcb_query_tree(conn, window);
           if ((reply = xcb_query_tree_reply(conn, cookie, NULL))) {
               printf("root = 0x%08x\n", reply->root);
               printf("parent = 0x%08x\n", reply->parent);

               xcb_window_t *children = xcb_query_tree_children(reply);
               for (int i = 0; i < xcb_query_tree_children_length(reply); i++)
                   printf("child window = 0x%08x\n", children[i]);

               free(reply);
           }
       }




    int 
    main ()
    {
        /* Open the connection to the X server. Use the DISPLAY environment variable */

        int i, screenNum;
        xcb_connection_t *connection = xcb_connect (NULL, &screenNum);


        /* Get the screen whose number is screenNum */

        const xcb_setup_t *setup = xcb_get_setup (connection);
        xcb_screen_iterator_t iter = xcb_setup_roots_iterator (setup);  

        // we want the screen at index screenNum of the iterator
        for (i = 0; i < screenNum; ++i) {
            xcb_screen_next (&iter);
        }

        xcb_screen_t *screen = iter.data;


        /* report */

        printf ("\n");
        printf ("Informations of screen %"PRIu32":\n", screen->root);
        printf ("  width.........: %"PRIu16"\n", screen->width_in_pixels);
        printf ("  height........: %"PRIu16"\n", screen->height_in_pixels);
        printf ("  white pixel...: %"PRIu32"\n", screen->white_pixel);
        printf ("  black pixel...: %"PRIu32"\n", screen->black_pixel);
        printf ("\n");

my_example(connection,screen->root);
        return 0;
    }

