#include <gtk/gtk.h>

typedef GtkApplication application;
typedef GtkApplicationClass applicationClass;

G_DEFINE_TYPE(application, application, GTK_TYPE_APPLICATION)

/*
 * Main window struct
 */
typedef struct {
    GtkApplicationWindow parent_instance;

    GtkWidget *message;
    GtkWidget *infobar;
    GtkWidget *status;
    GtkWidget *menutool;
    GMenuModel *toolmenu;
    GtkTextBuffer *buffer;

    int width;
    int height;
    gboolean maximized;
    gboolean fullscreen;
} applicationWindow;

/*
 * name simplification for gtappwindowclass
 */
typedef GtkApplicationWindowClass applicationWindowClass;

G_DEFINE_TYPE(applicationWindow, application_window, GTK_TYPE_APPLICATION_WINDOW)

static void create_window(GApplication *app, const char *contents);

/*
 * What is the action dialog?
 * Shows when menu are activated
 *
*/
static void show_action_dialog(GSimpleAction *action)
{
    const gchar *name;
    GtkWidget *dialog;

    name = g_action_get_name(G_ACTION(action));

    dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_INFO,
				   GTK_BUTTONS_CLOSE, "You activated action: \"%s\"", name);

    g_signal_connect(dialog, "response",
		    G_CALLBACK(gtk_widget_destroy), NULL);

    gtk_widget_show(dialog);
}

/*
 * TODO: What is this?
 * */
static void showaction_infobar(GSimpleAction *action, GVariant *parameter, gpointer data) {
    applicationWindow *window = data;
    gchar *text;
    const gchar *name;
    const gchar *value;

    name = g_action_get_name(G_ACTION(action));
    value = g_variant_get_string(parameter, NULL);
    text = g_strdup_printf("You activated radio action: \"%s\".\n Current value: %s", name, value);
    gtk_label_set_text(GTK_LABEL(window->message), text);
    gtk_widget_show(window->infobar);
    g_free(text);
}

static void activate_action(GSimpleAction *action, GVariant *parameter, gpointer user_data) {
  show_action_dialog(action);
}

static void activate_new(GSimpleAction *action, GVariant *parameter, gpointer user_data) {
    GApplication *app = user_data;

    create_window(app, NULL);
}

static void save_response_cb(GtkNativeDialog *dialog, gint response_id, gpointer user_data) {
    GtkFileChooserNative *native = user_data;
    GApplication *app = g_object_get_data(G_OBJECT(native), "app");
    GtkWidget *message_dialog;
    GFile *file;
    char *contents;
    GError *error = NULL;

    if(response_id == GTK_RESPONSE_ACCEPT) {
      file = gtk_file_chooser_get_file(GTK_FILE_CHOOSER(native));

      if(g_file_load_contents(file, NULL, &contents, NULL, NULL, &error)) {
          create_window(app, contents);
          g_free(contents);
      }
      else {
          message_dialog = gtk_message_dialog_new(
			    NULL,
                            GTK_DIALOG_DESTROY_WITH_PARENT,
                            GTK_MESSAGE_ERROR,
                            GTK_BUTTONS_CLOSE,
                            "Error loading file: \"%s\"",
                            error->message);
          g_signal_connect(message_dialog, "response",
                            G_CALLBACK(gtk_widget_destroy), NULL);
          gtk_widget_show(message_dialog);
          g_error_free(error);
        }
    }

    gtk_native_dialog_destroy(GTK_NATIVE_DIALOG(native));
    g_object_unref(native);
}
static void open_response_cb(GtkNativeDialog *dialog, gint response_id, gpointer user_data) {
    GtkFileChooserNative *native = user_data;
    GApplication *app = g_object_get_data(G_OBJECT(native), "app");
    GtkWidget *message_dialog;
    GFile *file;
    char *contents;
    GError *error = NULL;

    if(response_id == GTK_RESPONSE_ACCEPT) {
      file = gtk_file_chooser_get_file(GTK_FILE_CHOOSER(native));

      if(g_file_load_contents(file, NULL, &contents, NULL, NULL, &error)) {
          create_window(app, contents);
          g_free(contents);
        }
      else {
          message_dialog = gtk_message_dialog_new(
			    NULL,
                            GTK_DIALOG_DESTROY_WITH_PARENT,
                            GTK_MESSAGE_ERROR,
                            GTK_BUTTONS_CLOSE,
                            "Error loading file: \"%s\"",
                            error->message);
          g_signal_connect(message_dialog, "response",
                            G_CALLBACK(gtk_widget_destroy), NULL);
          gtk_widget_show(message_dialog);
          g_error_free(error);
        }
    }

    gtk_native_dialog_destroy(GTK_NATIVE_DIALOG(native));
    g_object_unref(native);
}

//TODO: Fix this 
static void activate_save(GSimpleAction *action, GVariant *parameter, gpointer user_data) {
    GApplication *app;
    GtkFileChooserNative *native;

    native = gtk_file_chooser_native_new(
    	"Open File",
        NULL,
        GTK_FILE_CHOOSER_ACTION_SAVE,
        "_Save",
       "_Cancel");

    g_object_set_data_full(G_OBJECT(native), "app", g_object_ref(app), g_object_unref);
    g_signal_connect(native,
		    "response",
		    G_CALLBACK(open_response_cb),
		    native);

//    gtk_native_dialog_run = user_data;


}
static void activate_open(GSimpleAction *action, GVariant *parameter, gpointer user_data) {
    GApplication *app = user_data;
    GtkFileChooserNative *native;

    native = gtk_file_chooser_native_new(
    	"Open File",
        NULL,
        GTK_FILE_CHOOSER_ACTION_OPEN,
        "_Open",
       "_Cancel");

    g_object_set_data_full(G_OBJECT(native), "app", g_object_ref(app), g_object_unref);
    g_signal_connect(native,
		    "response",
		    G_CALLBACK(open_response_cb),
		    native);

    gtk_native_dialog_show(GTK_NATIVE_DIALOG(native));
}

static void activate_toggle(GSimpleAction *action, GVariant *parameter, gpointer user_data) {
    GVariant *state;
    show_action_dialog(action);

    state = g_action_get_state(G_ACTION(action));
    g_action_change_state(G_ACTION(action), g_variant_new_boolean(!g_variant_get_boolean(state)));
    g_variant_unref(state);
}

static void activate_radio(GSimpleAction *action, GVariant *parameter, gpointer user_data) {
    showaction_infobar(action, parameter, user_data);
    g_action_change_state(G_ACTION(action), parameter);
}

static void activateabout(GSimpleAction *action, GVariant *parameter, gpointer user_data) {
    GtkWidget *window = user_data;
    const gchar *authors[] = { "Robert Kyrk", NULL};
    const gchar *documentors[] = {"Robert Kyrk", NULL};

    gtk_show_about_dialog(
	GTK_WINDOW(window),  
        "program-name",
        "GTK+ Code s",
        "version", 
        g_strdup_printf(",\nRunning against GTK+ %d.%d.%d",
        gtk_get_major_version(),
        gtk_get_minor_version(),
        gtk_get_micro_version()),
        "copyright", "(C) 1997-2013 The GTK+ Team",
        "license-type", GTK_LICENSE_LGPL_2_1,
        "website", "http://www.gtk.org",
        "comments", "Program to nstrate GTK+ functions.",
        "authors", authors,
        "documenters", documentors,
        "logo-icon-name", "gtk3-",
	"title", "About GTK+ Code s",
	NULL);
}

static void activate_quit(GSimpleAction *action, GVariant *parameter, gpointer user_data) {
    GtkApplication *app = user_data;
    GtkWidget *win;
    GList *list, *next;

    list = gtk_application_get_windows(app);
    while(list)
    {
      win = list->data;
      next = list->next;

      gtk_widget_destroy(GTK_WIDGET(win));

      list = next;
    }
}

static void update_statusbar(GtkTextBuffer *buffer, applicationWindow *window) {
    gchar *msg;
    gint row, col;
    gint count;
    GtkTextIter iter;

    /* clear any previous message, underflow is allowed */
    gtk_statusbar_pop(GTK_STATUSBAR(window->status), 0);

    count = gtk_text_buffer_get_char_count(buffer);

    gtk_text_buffer_get_iter_at_mark(buffer,
				    &iter,
				    gtk_text_buffer_get_insert(buffer));

    row = gtk_text_iter_get_line(&iter);
    col = gtk_text_iter_get_line_offset(&iter);

    msg = g_strdup_printf("Cursor at row %d column %d - %d chars in document",
			 row, col, count);

    gtk_statusbar_push(GTK_STATUSBAR(window->status), 0, msg);

    g_free(msg);
}

static void mark_set_callback(GtkTextBuffer *buffer, const GtkTextIter *new_location, GtkTextMark *mark, applicationWindow *window) {
    update_statusbar(buffer, window);
}

static void change_theme_state(GSimpleAction *action, GVariant *state, gpointer user_data) {
    GtkSettings *settings = gtk_settings_get_default();

    g_object_set(G_OBJECT(settings),
                "gtk-application-prefer-dark-theme",
                g_variant_get_boolean(state),
                NULL);

    g_simple_action_set_state(action, state);
}

static void change_titlebar_state(GSimpleAction *action, GVariant *state, gpointer user_data) {
  GtkWindow *window = user_data;

  gtk_window_set_hide_titlebar_when_maximized(GTK_WINDOW(window),
                                               g_variant_get_boolean(state));

  g_simple_action_set_state(action, state);
}

static void change_radio_state(GSimpleAction *action, GVariant *state, gpointer user_data) {
  g_simple_action_set_state(action, state);
}

static GActionEntry app_entries[] = {
  { "new", activate_new, NULL, NULL, NULL },
  { "open", activate_open, NULL, NULL, NULL },  
  { "save", activate_save, NULL, NULL, NULL }, //TODO
  { "save-as", activate_action, NULL, NULL, NULL }, //TODO
  { "quit", activate_quit, NULL, NULL, NULL },
  { "dark", activate_toggle, NULL, "false", change_theme_state }
};

static GActionEntry win_entries[] = {
  { "titlebar", activate_toggle, NULL, "false", change_titlebar_state },
  { "shape", activate_radio, "s", "'oval'", change_radio_state },
  { "bold", activate_toggle, NULL, "false", NULL },
  { "about", activateabout, NULL, NULL, NULL },
  { "file1", activate_action, NULL, NULL, NULL },
  { "logo", activate_action, NULL, NULL, NULL }
};

static void clicked_cb(GtkWidget *widget, applicationWindow *window) {
  gtk_widget_hide(window->infobar);
}

/*
 * init function?
 */
static void startup(GApplication *app) {
  GtkBuilder *builder;
  GMenuModel *appmenu;
  GMenuModel *menubar;

  G_APPLICATION_CLASS(application_parent_class)->startup(app);

  builder = gtk_builder_new();
  gtk_builder_add_from_resource(builder, "/menus/menus.ui", NULL);

  appmenu =(GMenuModel *)gtk_builder_get_object(builder, "appmenu");
  menubar =(GMenuModel *)gtk_builder_get_object(builder, "menubar");

  gtk_application_set_app_menu(GTK_APPLICATION(app), appmenu);
  gtk_application_set_menubar(GTK_APPLICATION(app), menubar);

  g_object_unref(builder);
}

static void create_window(GApplication *app, const char *content) {
  applicationWindow *window;

  window = g_object_new(application_window_get_type(),
                                                  "application", app,
                                                  NULL);
  if(content)
    gtk_text_buffer_set_text(window->buffer, content, -1);

  gtk_window_present(GTK_WINDOW(window));
}

static void activate(GApplication *app) {
  create_window(app, NULL);
}

static void application_init(application *app) {
  GSettings *settings;
  GAction *action;

  settings = g_settings_new("org.gtk.Demo");

  g_action_map_add_action_entries(G_ACTION_MAP(app),
                                   app_entries, G_N_ELEMENTS(app_entries),
                                   app);

  action = g_settings_create_action(settings, "color");

  g_action_map_add_action(G_ACTION_MAP(app), action);

  g_object_unref(settings);
}

static void application_class_init(applicationClass *class) {
  GApplicationClass *app_class = G_APPLICATION_CLASS(class);

  app_class->startup = startup;
  app_class->activate = activate;
}

static void application_window_store_state(applicationWindow *win) {
  GSettings *settings;

  settings = g_settings_new("org.gtk.Demo");
  g_settings_set(settings, "window-size", "(ii)", win->width, win->height);
  g_settings_set_boolean(settings, "maximized", win->maximized);
  g_settings_set_boolean(settings, "fullscreen", win->fullscreen);
  g_object_unref(settings);
}

static void application_window_load_state(applicationWindow *win) {
  GSettings *settings;

  settings = g_settings_new("org.gtk.Demo");
  g_settings_get(settings, "window-size", "(ii)", &win->width, &win->height);
  win->maximized = g_settings_get_boolean(settings, "maximized");
  win->fullscreen = g_settings_get_boolean(settings, "fullscreen");
  g_object_unref(settings);
}

static void application_window_init(applicationWindow *window) {
  GtkWidget *menu;

  window->width = -1;
  window->height = -1;
  window->maximized = FALSE;
  window->fullscreen = FALSE;

  gtk_widget_init_template(GTK_WIDGET(window));

  menu = gtk_menu_new_from_model(window->toolmenu);
  gtk_menu_tool_button_set_menu(GTK_MENU_TOOL_BUTTON(window->menutool), menu);

  g_action_map_add_action_entries(G_ACTION_MAP(window),
                                   win_entries, G_N_ELEMENTS(win_entries),
                                   window);
}

static void application_window_constructed(GObject *object) {
  applicationWindow *window =(applicationWindow *)object;

  application_window_load_state(window);

  gtk_window_set_default_size(GTK_WINDOW(window), window->width, window->height);

  if(window->maximized)
    gtk_window_maximize(GTK_WINDOW(window));

  if(window->fullscreen)
    gtk_window_fullscreen(GTK_WINDOW(window));

  G_OBJECT_CLASS(application_window_parent_class)->constructed(object);
}

static void application_window_size_allocate(GtkWidget *widget, GtkAllocation *allocation) {
  applicationWindow *window =(applicationWindow *)widget;

  GTK_WIDGET_CLASS(application_window_parent_class)->size_allocate(widget, allocation);

  if(!window->maximized && !window->fullscreen)
    gtk_window_get_size(GTK_WINDOW(window), &window->width, &window->height);
}

static gboolean application_window_state_event(GtkWidget *widget, GdkEventWindowState *event) {
  applicationWindow *window =(applicationWindow *)widget;
  gboolean res = GDK_EVENT_PROPAGATE;

  if(GTK_WIDGET_CLASS(application_window_parent_class)->window_state_event)
    res = GTK_WIDGET_CLASS(application_window_parent_class)->window_state_event(widget, event);

  window->maximized =(event->new_window_state & GDK_WINDOW_STATE_MAXIMIZED) != 0;
  window->fullscreen =(event->new_window_state & GDK_WINDOW_STATE_FULLSCREEN) != 0;

  return res;
}

static void application_window_destroy(GtkWidget *widget) {
  applicationWindow *window =(applicationWindow *)widget;

  application_window_store_state(window);

  GTK_WIDGET_CLASS(application_window_parent_class)->destroy(widget);
}

static void application_window_class_init(applicationWindowClass *class) {
  GObjectClass *object_class = G_OBJECT_CLASS(class);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(class);

  object_class->constructed = application_window_constructed;

  widget_class->size_allocate = application_window_size_allocate;
  widget_class->window_state_event = application_window_state_event;
  widget_class->destroy = application_window_destroy;

  gtk_widget_class_set_template_from_resource(widget_class, "/application/application.ui");
  gtk_widget_class_bind_template_child(widget_class, applicationWindow, message);
  gtk_widget_class_bind_template_child(widget_class, applicationWindow, infobar);
  gtk_widget_class_bind_template_child(widget_class, applicationWindow, status);
  gtk_widget_class_bind_template_child(widget_class, applicationWindow, buffer);
  gtk_widget_class_bind_template_child(widget_class, applicationWindow, menutool);
  gtk_widget_class_bind_template_child(widget_class, applicationWindow, toolmenu);
  gtk_widget_class_bind_template_callback(widget_class, clicked_cb);
  gtk_widget_class_bind_template_callback(widget_class, update_statusbar);
  gtk_widget_class_bind_template_callback(widget_class, mark_set_callback);
}

int main(int argc, char *argv[]) {
  GtkApplication *app;

  app = GTK_APPLICATION(g_object_new(application_get_type(),
                                       "application-id", "Kyrk Editor",
                                       "flags", G_APPLICATION_HANDLES_OPEN,
                                       NULL));

  return g_application_run(G_APPLICATION(app), 0, NULL);
}
