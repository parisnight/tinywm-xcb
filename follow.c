/* follow mouse wm  2020.10.15 */

#include <xcb/xcb.h>
#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
  uint32_t values[3], button;
  xcb_connection_t *dpy;
  xcb_screen_t *screen;  /* screen->width_in_pixels */
  xcb_drawable_t win, root;
  xcb_get_geometry_reply_t *geom;
  xcb_query_pointer_reply_t *pointer, *pointer1;
  xcb_generic_event_t *ev;
  xcb_button_press_event_t *e;
  xcb_enter_notify_event_t *enter; 
  xcb_create_notify_event_t *create;
  const uint32_t select_input_val[] = {
    /* good grief, new xapps don't appear
        XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT | */
        XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY |
        XCB_EVENT_MASK_ENTER_WINDOW |
        XCB_EVENT_MASK_LEAVE_WINDOW | XCB_EVENT_MASK_STRUCTURE_NOTIFY |
        XCB_EVENT_MASK_PROPERTY_CHANGE |
        XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE
        | XCB_EVENT_MASK_FOCUS_CHANGE
  };

  dpy = xcb_connect(NULL, NULL);
  if (xcb_connection_has_error(dpy)) return(1);

  screen = xcb_setup_roots_iterator(xcb_get_setup(dpy)).data;
  root = screen->root;

  xcb_grab_button(dpy, 0, root, XCB_EVENT_MASK_BUTTON_PRESS |
                  XCB_EVENT_MASK_BUTTON_RELEASE, XCB_GRAB_MODE_ASYNC,
                  XCB_GRAB_MODE_ASYNC, root, XCB_NONE, 1, XCB_MOD_MASK_1);

  xcb_grab_button(dpy, 0, root, XCB_EVENT_MASK_BUTTON_PRESS |
                  XCB_EVENT_MASK_BUTTON_RELEASE, XCB_GRAB_MODE_ASYNC,
                  XCB_GRAB_MODE_ASYNC, root, XCB_NONE, 3, XCB_MOD_MASK_1);
  xcb_change_window_attributes(dpy, root,
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
      create = (xcb_create_notify_event_t *) ev;
      printf("Created window %" PRIu32 " \n", create->window);
      values[0] = XCB_EVENT_MASK_PROPERTY_CHANGE
          | XCB_EVENT_MASK_ENTER_WINDOW;
      xcb_change_window_attributes_checked(dpy, create->window,
                                           XCB_CW_EVENT_MASK, values);
      break;
    case XCB_DESTROY_NOTIFY:
      printf("destroy notify\n");
      break;
    case XCB_EXPOSE:
      printf("xcb expose\n");
      break;

    case XCB_ENTER_NOTIFY:
      enter = (xcb_enter_notify_event_t *) ev;
      printf("Enter window %d (%d %d)\n",
              enter->event, enter->event_x, enter->event_y);
      xcb_set_input_focus(dpy, XCB_INPUT_FOCUS_POINTER_ROOT,
                          enter->event, XCB_CURRENT_TIME);
      break;

    case XCB_LEAVE_NOTIFY:
      printf("leave notify\n");
      //xcb_set_input_focus (dpy, XCB_INPUT_FOCUS_NONE, win, XCB_CURRENT_TIME);
      break;

    case XCB_BUTTON_PRESS:
      printf("press\n");
      e = (xcb_button_press_event_t *) ev;
      win = e->child;
      button = e->detail;
      values[0] = XCB_STACK_MODE_ABOVE;
      xcb_configure_window(dpy, win, XCB_CONFIG_WINDOW_STACK_MODE, values);
      pointer1 =
            xcb_query_pointer_reply(dpy, xcb_query_pointer(dpy, root), 0);
      geom = xcb_get_geometry_reply(dpy, xcb_get_geometry(dpy, win), NULL);
      xcb_grab_pointer(dpy, 0, root, XCB_EVENT_MASK_BUTTON_RELEASE
                       | XCB_EVENT_MASK_BUTTON_MOTION |
                       XCB_EVENT_MASK_POINTER_MOTION_HINT,
                       XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC,
                       root, XCB_NONE, XCB_CURRENT_TIME);
      break;

    case XCB_MOTION_NOTIFY: /* need to grab_pointer in button press */
      printf("motion notify\n");
      pointer =
          xcb_query_pointer_reply(dpy, xcb_query_pointer(dpy, root), 0);
      if (button == 1) {       /* move */
        values[0] = geom->x + pointer->root_x - pointer1->root_x;
        values[1] = geom->y + pointer->root_y - pointer1->root_y;
        printf("motion_nofify %d %d\n", values[0], values[1]);
        xcb_configure_window(dpy, win,
                             XCB_CONFIG_WINDOW_X |
                             XCB_CONFIG_WINDOW_Y, values);
      } else if (button == 3) {        /* resize */
        values[0] = geom->width + pointer->root_x - pointer1->root_x;
        values[1] = geom->height + pointer->root_y - pointer1->root_y;
        xcb_configure_window(dpy, win,
                             XCB_CONFIG_WINDOW_WIDTH |
                             XCB_CONFIG_WINDOW_HEIGHT, values);
      }
      break;

    case XCB_BUTTON_RELEASE:
      printf("button release\n");
      xcb_set_input_focus(dpy, XCB_INPUT_FOCUS_POINTER_ROOT, win,
                          XCB_CURRENT_TIME);
      xcb_ungrab_pointer(dpy, XCB_CURRENT_TIME);
      break;

    default:
      printf("Default event handler response_type %d\n", ev->response_type);
    }
    xcb_flush(dpy);
  }
  return 0;
}
