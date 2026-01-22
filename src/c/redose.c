#include <pebble.h>

static Window* s_window;
static TextLayer* s_text_layer;

static void prv_exit_application(void* data) {
  exit_reason_set(APP_EXIT_ACTION_PERFORMED_SUCCESSFULLY);
  // Exit the application by unloading the only window
  window_stack_remove(s_window, false);
}

static void prv_exit_delay() {
  int timeout = preferred_result_display_duration();
  AppTimer* timer = app_timer_register(timeout, prv_exit_application, NULL);
}

static void prv_window_load(Window* window) {
  Layer* window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_text_layer = text_layer_create(GRect(0, (bounds.size.h / 2) - 30, bounds.size.w, 60));
  text_layer_set_text(s_text_layer, "miaows at u");
  text_layer_set_text_alignment(s_text_layer, GTextAlignmentCenter);
  text_layer_set_font(s_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  layer_add_child(window_layer, text_layer_get_layer(s_text_layer));
}

static void prv_window_unload(Window* window) {
  text_layer_destroy(s_text_layer);
}

// https://developer.repebble.com/guides/user-interfaces/appglance-c/

static void prv_update_app_glance(AppGlanceReloadSession* session, size_t limit, void* context) {
  // Check we haven't exceeded system limit of AppGlance's
  if (limit < 1) return;

  const char* subtitle = (const char*)context;
  char str[50];
  snprintf(str, sizeof str, "%s", subtitle);

  // Create the AppGlanceSlice (no icon, no expiry)
  AppGlanceSlice entry = (AppGlanceSlice){
    .layout = {
      .subtitle_template_string = "miaow"
    },
    .expiration_time = time(NULL) + 3600
  };

  time_t now = time(NULL);
  APP_LOG(APP_LOG_LEVEL_INFO, "now=%ld", (long)now);
  entry.expiration_time = now + 3600;

  // Add the slice, and check the result
  const AppGlanceResult result = app_glance_add_slice(session, entry);
  if (result != APP_GLANCE_RESULT_SUCCESS) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "AppGlance Error: %d", result);
  }
}

static void prv_init(void) {
  s_window = window_create();

  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = prv_window_load,
      .unload = prv_window_unload,
  });
  window_stack_push(s_window, false);

  prv_exit_delay();
}

static void prv_deinit(void) {
  window_destroy(s_window);
  app_glance_reload(prv_update_app_glance, "miaow");
}

int main(void) {
  prv_init();
  app_event_loop();
  prv_deinit();
}
