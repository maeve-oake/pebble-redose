#include <pebble.h>
#define NUM_STATES 4

const long ritalin_duration[NUM_STATES] = {
  2400, // 40m onset
  4800, // + 40m come-up
  11100, // + 1hr 45m peak
  15600 // + 1hr 15m comedown
};

const char* messages[NUM_STATES] = {
  "onset",
  "come-up",
  "peak",
  "come-down"
};

static Window* s_window;
static TextLayer* s_text_layer;
static Layer* s_canvas_layer;
static GDrawCommandSequence* s_command_seq;

// static void exit_application(void* data) {
//   exit_reason_set(APP_EXIT_ACTION_PERFORMED_SUCCESSFULLY);
//   // Exit the application by unloading the only window
//   window_stack_remove(s_window, false);
// }

// static void exit_delay() {
//   int timeout = preferred_result_display_duration();
//   AppTimer* timer = app_timer_register(timeout, exit_application, NULL);
// }

// ms between frames
#define DELTA 30 
static int s_index = 0;

static void update_proc(Layer* layer, GContext* ctx) {
  // get next frame
  GDrawCommandFrame* frame = gdraw_command_sequence_get_frame_by_index(s_command_seq, s_index);

  // if another frame, draw
  if (frame) {
    gdraw_command_frame_draw(ctx, s_command_seq, frame, GPoint(0, 30));
  }

  // advance to next frame, wrap if needed
  int num_frames = gdraw_command_sequence_get_num_frames(s_command_seq);
  s_index++;
  if (s_index == num_frames) {
    s_index = 0;
  };
};

static void window_load(Window* window) {
  Layer* window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  window_set_background_color(window, GColorVividCerulean);

  s_text_layer = text_layer_create(GRect(0, (bounds.size.h / 2), bounds.size.w, 30));
  text_layer_set_text(s_text_layer, "redosed ritalin!");
  text_layer_set_background_color(s_text_layer, GColorClear);
  text_layer_set_text_alignment(s_text_layer, GTextAlignmentCenter);
  text_layer_set_font(s_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  layer_add_child(window_layer, text_layer_get_layer(s_text_layer));

  s_canvas_layer = layer_create(GRect((bounds.size.w / 3), 0, bounds.size.w, bounds.size.h));
  layer_set_update_proc(s_canvas_layer, update_proc);
  layer_add_child(window_layer, s_canvas_layer);
}

static void window_unload(Window* window) {
  text_layer_destroy(s_text_layer);
  layer_destroy(s_canvas_layer);
  gdraw_command_sequence_destroy(s_command_seq);
}

// https://developer.repebble.com/guides/user-interfaces/appglance-c/

static void update_app_glance(AppGlanceReloadSession* session, size_t limit, void* context) {
  // Check we haven't exceeded system limit of AppGlance's
  if (limit < 1) return;

  time_t expiration_time = time(NULL);

  // iterate through ritalin durations
  for (int i = 0; i < NUM_STATES; i++) {
    char subtitle[64];
    snprintf(subtitle, sizeof subtitle, "%s {time_until(%ld)|format('%%aT')}", messages[i], expiration_time + ritalin_duration[i]);

    AppGlanceSlice entry = (AppGlanceSlice){
      .layout = {
        .subtitle_template_string = subtitle
      },
      .expiration_time = expiration_time + ritalin_duration[i]
    };

    // Add the slice, and check the result
    const AppGlanceResult result = app_glance_add_slice(session, entry);
    if (result != APP_GLANCE_RESULT_SUCCESS) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "AppGlance Error: %d", result);
    }
  }
}

static void next_frame_handler(void* context) {
  layer_mark_dirty(s_canvas_layer);
  app_timer_register(DELTA, next_frame_handler, NULL);
}

static void init(void) {
  s_window = window_create();
  s_command_seq = gdraw_command_sequence_create_with_resource(RESOURCE_ID_CLOCK_SEQUENCE);

  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = window_load,
      .unload = window_unload,
  });
  window_stack_push(s_window, false);

  app_glance_reload(update_app_glance, NULL);

  app_timer_register(DELTA, next_frame_handler, NULL);

  // exit_delay();
}

static void deinit(void) {
  window_destroy(s_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
