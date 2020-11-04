/* follow mouse wm  2020.10.15 */

#include <xcb/xcb.h>
#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    uint32_t values[3];

    xcb_connection_t *dpy;
    xcb_screen_t *screen;
    xcb_drawable_t win;
    xcb_drawable_t root;

    xcb_generic_event_t *ev;
    xcb_get_geometry_reply_t *geom;

    dpy = xcb_connect(NULL, NULL);
    if (xcb_connection_has_error(dpy))
	return 1;

    screen = xcb_setup_roots_iterator(xcb_get_setup(dpy)).data;
    root = screen->root;

    //  xcb_grab_key (dpy, 1, root, XCB_MOD_MASK_2, XCB_NO_SYMBOL,
    //            XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);

    xcb_grab_button(dpy, 0, root, XCB_EVENT_MASK_BUTTON_PRESS |
		    XCB_EVENT_MASK_BUTTON_RELEASE, XCB_GRAB_MODE_ASYNC,
		    XCB_GRAB_MODE_ASYNC, root, XCB_NONE, 1,
		    XCB_MOD_MASK_1);

    xcb_grab_button(dpy, 0, root, XCB_EVENT_MASK_BUTTON_PRESS |
		    XCB_EVENT_MASK_BUTTON_RELEASE, XCB_GRAB_MODE_ASYNC,
		    XCB_GRAB_MODE_ASYNC, root, XCB_NONE, 3,
		    XCB_MOD_MASK_1);

    const uint32_t select_input_val[] = {
    /*** good grief, new xapps don't appear
    XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT |
      */ XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY |
	    XCB_EVENT_MASK_ENTER_WINDOW |
	XCB_EVENT_MASK_LEAVE_WINDOW | XCB_EVENT_MASK_STRUCTURE_NOTIFY |
	    XCB_EVENT_MASK_PROPERTY_CHANGE |
	    XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE
	    | XCB_EVENT_MASK_FOCUS_CHANGE
    };
    xcb_change_window_attributes(dpy, screen->root,
				 XCB_CW_EVENT_MASK, select_input_val);
    xcb_flush(dpy);

    while (1) {
	ev = xcb_wait_for_event(dpy);
	switch ((ev->response_type) & ~0x80) {
	case XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT:
	case XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY:
	    printf("substructure redirect or notify\n");
	    break;
	case XCB_CREATE_NOTIFY:
	    printf("create notify\n");
	    xcb_create_notify_event_t *enter =
		(xcb_create_notify_event_t *) ev;
	    printf("Mouse entered window %" PRIu32 " \n", enter->window);

	    unsigned int valuet[1] =
		{ XCB_EVENT_MASK_PROPERTY_CHANGE |
(XCB_EVENT_MASK_ENTER_WINDOW) };
	    xcb_change_window_attributes_checked(dpy, enter->window,
						 XCB_CW_EVENT_MASK,
						 valuet);
	    break;
	case XCB_DESTROY_NOTIFY:
	    printf("destroy notify\n");
	    break;
	case XCB_EXPOSE:
	    printf("xcb expose\n");
	    break;

	case XCB_ENTER_NOTIFY:{
		xcb_enter_notify_event_t *enter =
		    (xcb_enter_notify_event_t *) ev;
		printf("Mouse entered window %" PRIu32
		       ", at coordinates (%" PRIi16 ",%" PRIi16 ")\n",
		       enter->event, enter->event_x, enter->event_y);
		xcb_set_input_focus(dpy, XCB_INPUT_FOCUS_POINTER_ROOT,
				    enter->event, XCB_CURRENT_TIME);
	    }
	    break;

	case XCB_LEAVE_NOTIFY:
	    printf("leave notify\n");
	    //xcb_set_input_focus (dpy, XCB_INPUT_FOCUS_NONE, win, XCB_CURRENT_TIME);
	    break;

	case XCB_BUTTON_PRESS:
	    printf("press\n");
	    xcb_button_press_event_t *e;
	    e = (xcb_button_press_event_t *) ev;
	    win = e->child;
	    values[0] = XCB_STACK_MODE_ABOVE;
	    xcb_configure_window(dpy, win, XCB_CONFIG_WINDOW_STACK_MODE,
				 values);
	    geom =
		xcb_get_geometry_reply(dpy, xcb_get_geometry(dpy, win),
				       NULL);
	    if (1 == e->detail) {
		values[2] = 1;
		xcb_warp_pointer(dpy, XCB_NONE, win, 0, 0, 0, 0, 1, 1);
	    } else {
		values[2] = 3;
		xcb_warp_pointer(dpy, XCB_NONE, win, 0, 0, 0, 0,
				 geom->width, geom->height);
	    }
	    xcb_grab_pointer(dpy, 0, root, XCB_EVENT_MASK_BUTTON_RELEASE
			     | XCB_EVENT_MASK_BUTTON_MOTION |
			     XCB_EVENT_MASK_POINTER_MOTION_HINT,
			     XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC,
			     root, XCB_NONE, XCB_CURRENT_TIME);
	    break;

	case XCB_MOTION_NOTIFY:
	    printf("motion notify\n");
	    xcb_query_pointer_reply_t *pointer;
	    pointer =
		xcb_query_pointer_reply(dpy, xcb_query_pointer(dpy, root),
					0);
	    if (values[2] == 1) {	/* move */
		geom =
		    xcb_get_geometry_reply(dpy, xcb_get_geometry(dpy, win),
					   NULL);
		values[0] =
		    (pointer->root_x + geom->width >
		     screen->width_in_pixels) ? (screen->width_in_pixels -
						 geom->width) : pointer->
		    root_x;
		values[1] =
		    (pointer->root_y + geom->height >
		     screen->height_in_pixels) ? (screen->
						  height_in_pixels -
						  geom->height) : pointer->
		    root_y;
		xcb_configure_window(dpy, win,
				     XCB_CONFIG_WINDOW_X |
				     XCB_CONFIG_WINDOW_Y, values);
	    } else if (values[2] == 3) {	/* resize */
		geom =
		    xcb_get_geometry_reply(dpy, xcb_get_geometry(dpy, win),
					   NULL);
		values[0] = pointer->root_x - geom->x;
		values[1] = pointer->root_y - geom->y;
		xcb_configure_window(dpy, win,
				     XCB_CONFIG_WINDOW_WIDTH |
				     XCB_CONFIG_WINDOW_HEIGHT, values);
	    }
	    break;

	case XCB_BUTTON_RELEASE:
	    printf("releasepooh\n");
	    xcb_set_input_focus(dpy, XCB_INPUT_FOCUS_POINTER_ROOT, win,
				XCB_CURRENT_TIME);
	    //xcb_set_input_focus (dpy, XCB_INPUT_FOCUS_NONE, win, XCB_CURRENT_TIME);
	    xcb_ungrab_pointer(dpy, XCB_CURRENT_TIME);
	    break;

	default:
	    printf("response_type %d\n", ev->response_type);
	}
	xcb_flush(dpy);
    }
    return 0;
}


