#include <pebble.h>
#include <pebble-events/pebble-events.h>
#include <pebble-generic-weather/pebble-generic-weather.h>

static Window *s_window;
static Layer *s_window_layer, *s_dots_layer, *s_progress_layer, *s_average_layer, *s_battery_layer, *s_bt_layer;
static TextLayer *s_time_layer, *s_step_layer, *s_date_layer, *s_status_layer;
static char s_current_time_buffer[8], s_current_steps_buffer[16], StringUserMidStepGoal[6], StringUserStepGoal[6];
static int s_step_count = 1, s_step_count_prev =1, s_step_goal = 1, s_step_average = 1, s_battery_level, s_battery_charging, UserMidStepGoal = 4500, UserStepGoal = 7500;
#define TIMER_INTERVAL_MS 180000
static char generic_status[]="Day Avg:\n10,000";

static void read_persist()
{
	if(persist_exists(MESSAGE_KEY_STEPGOAL))
	{
		persist_read_string(MESSAGE_KEY_STEPGOAL, StringUserStepGoal, sizeof(StringUserStepGoal));
    if (strlen(StringUserStepGoal)>0) {UserStepGoal = atoi(StringUserStepGoal);}
	}
	if(persist_exists(MESSAGE_KEY_STEPMIDGOAL))
	{
		persist_read_string(MESSAGE_KEY_STEPMIDGOAL, StringUserMidStepGoal, sizeof(StringUserMidStepGoal));
    if (strlen(StringUserMidStepGoal)>0){UserMidStepGoal = atoi(StringUserMidStepGoal);}
	}
}

static void store_persist()
{
  persist_write_string(MESSAGE_KEY_STEPGOAL, StringUserStepGoal);
  persist_write_string(MESSAGE_KEY_STEPMIDGOAL, StringUserMidStepGoal);
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context)
{
	Tuple *data = dict_find(iterator, MESSAGE_KEY_READY);
	if(data)
	{
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Ready Received!.");
  }
  
	data = dict_find(iterator, MESSAGE_KEY_STEPGOAL);
	if(data)
	{
		strcpy(StringUserStepGoal, data->value->cstring);
    UserStepGoal = atoi(StringUserStepGoal);
	}

	data = dict_find(iterator, MESSAGE_KEY_STEPMIDGOAL);
	if(data)
	{
		strcpy(StringUserMidStepGoal, data->value->cstring);
    UserMidStepGoal = atoi(StringUserMidStepGoal);
	}
}

static void inbox_dropped_callback(AppMessageResult reason, void *context)
{
	APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context)
{
	APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context)
{
	APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

static int get_health(HealthMetric metric, int average)
{
  //HealthMetricStepCount
  //HealthMetricSleepSeconds
	time_t start = time_start_of_today();
	time_t end = time(NULL);

	int output =0;
  switch(average) {
	case 0: {
		HealthServiceAccessibilityMask mask = health_service_metric_accessible(metric, start, end);
			if(mask & HealthServiceAccessibilityMaskAvailable)
		{
			//APP_LOG(APP_LOG_LEVEL_INFO, "Metric: %d", (int)health_service_sum_today(metric));
			output = health_service_sum_today(metric);
		}
    else {
			APP_LOG(APP_LOG_LEVEL_ERROR, "Health metric unavaiilable");
      output = 0;
    }
    break;    
  }
  case 1: {
    // Check that an averaged value is accessible
    HealthServiceAccessibilityMask mask = 
          health_service_metric_averaged_accessible(metric, start, end, HealthServiceTimeScopeWeekly);
    if(mask & HealthServiceAccessibilityMaskAvailable) {
      // Average is available, read it
      output = (int)health_service_sum_averaged(metric, start, end, HealthServiceTimeScopeWeekly);
      }
//    output = (int)health_service_sum_averaged(metric, start, end, HealthServiceTimeScopeDaily);
    break;
  }
  case 2: {
    HealthServiceAccessibilityMask mask = 
          health_service_metric_averaged_accessible(metric, start, start+SECONDS_PER_DAY, HealthServiceTimeScopeWeekly);
    if(mask & HealthServiceAccessibilityMaskAvailable) {
      // Average is available, read it
      output = (int)health_service_sum_averaged(metric, start, start+SECONDS_PER_DAY, HealthServiceTimeScopeWeekly);
      }
    break;
  }
	default:
		{
			APP_LOG(APP_LOG_LEVEL_ERROR, "Haalth data unavailable!");
		}
	}
	return output;
}

static void display_step_count() {
  int thousands = s_step_count / 1000;
  int hundreds = s_step_count % 1000;

  text_layer_set_text_color(s_step_layer, GColorPictonBlue);

  if(thousands > 0) {
    snprintf(s_current_steps_buffer, sizeof(s_current_steps_buffer),
      "%d,%03d steps", thousands, hundreds);
  } else {
    snprintf(s_current_steps_buffer, sizeof(s_current_steps_buffer),
      "%d steps", hundreds);
  }
  text_layer_set_text(s_step_layer, s_current_steps_buffer);
}

static void clear_status(){
  int thousands = s_step_goal / 1000;
  int hundreds = s_step_goal % 1000;
  if(thousands > 0) {
    snprintf(generic_status, sizeof(generic_status), "Day Avg:\n%d,%03d", thousands, hundreds);
  } else {
    snprintf(generic_status, sizeof(generic_status), "Day Avg:\n%d", s_step_goal);
  }
  text_layer_set_text(s_status_layer, generic_status);
}

static void update_time() {
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  strftime(s_current_time_buffer, sizeof(s_current_time_buffer),
           clock_is_24h_style() ? "%H:%M" : "%l:%M", tick_time);
//  snprintf(s_current_time_buffer,sizeof(s_current_time_buffer),"%d",UserMidStepGoal);
//  text_layer_set_text(s_time_layer, "12:44");
  text_layer_set_text(s_time_layer, s_current_time_buffer);
  static char s_weekdayname[30];
  strftime(s_weekdayname, sizeof(s_weekdayname), "%A%n%b %e", tick_time);
  text_layer_set_text(s_date_layer, s_weekdayname);
  s_step_count_prev = s_step_count;
  s_step_count = get_health(HealthMetricStepCount, 0);
  s_step_average = get_health(HealthMetricStepCount, 1);
  s_step_goal = get_health(HealthMetricStepCount, 2);
  s_step_count = 3298;
  s_step_average = 2398;
  if (s_step_count_prev <=1) {s_step_count_prev = s_step_count;}
  display_step_count();
  layer_mark_dirty(s_progress_layer);
  layer_mark_dirty(s_average_layer);
  if ((s_step_count_prev<UserStepGoal)&&(s_step_count>=UserStepGoal)) {
    static const uint32_t const segments[] = { 200, 200, 50, 50, 50, 50, 100, 200, 400 };
    VibePattern pat = {
      .durations = segments,
      .num_segments = ARRAY_LENGTH(segments),
    };
    vibes_enqueue_custom_pattern(pat);
    text_layer_set_text(s_status_layer, "Day Goal\nMet");
    app_timer_register(TIMER_INTERVAL_MS, clear_status, NULL);
  }
  else if ((s_step_count_prev<UserMidStepGoal)&&(s_step_count>=UserMidStepGoal)) {
    static const uint32_t const segments2[] = { 200, 200, 50, 50, 100, 200, 400 };
    VibePattern pat = {
      .durations = segments2,
      .num_segments = ARRAY_LENGTH(segments2),
    };
    vibes_enqueue_custom_pattern(pat);
    text_layer_set_text(s_status_layer, "Mid Goal\nMet");
    app_timer_register(TIMER_INTERVAL_MS, clear_status, NULL);
  }
  else if ((s_step_count_prev<s_step_goal)&&(s_step_count>=s_step_goal)) {
    static const uint32_t const segments2[] = { 200, 200, 200, 400 };
    VibePattern pat = {
      .durations = segments2,
      .num_segments = ARRAY_LENGTH(segments2),
    };
    vibes_enqueue_custom_pattern(pat);
    text_layer_set_text(s_status_layer, "Avg Goal\nMet");
    app_timer_register(TIMER_INTERVAL_MS, clear_status, NULL);
  }
}

static void tick_handler(struct tm *tick_time, TimeUnits changed) {
  update_time();
}

static void dots_layer_update_proc(Layer *layer, GContext *ctx) {
  const GRect inset = grect_inset(layer_get_bounds(layer), GEdgeInsets(6));

  const int num_dots = 12;
  for(int i = 0; i < num_dots; i++) {
    GPoint pos = gpoint_from_polar(inset, GOvalScaleModeFitCircle,
      DEG_TO_TRIGANGLE(i * 360 / num_dots));
    graphics_context_set_fill_color(ctx, GColorDarkGray);
    graphics_fill_circle(ctx, pos, 2);
  }
}

static void bluetooth_callback(bool connected) {
  layer_mark_dirty(s_bt_layer);
  if(!connected) {
    // Issue a vibrating alert
    vibes_double_pulse();
    layer_mark_dirty(s_bt_layer);
  }
}

static void battery_callback(BatteryChargeState state) {
  // Record the new battery level
  s_battery_level = state.charge_percent;
  s_battery_charging = state.is_charging;
  // Update meter
  layer_mark_dirty(s_battery_layer);
}

static void battery_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_draw_rect(ctx,GRect(0, 0, bounds.size.w-2, bounds.size.h));
  graphics_draw_line(ctx, GPoint(bounds.size.w-1, 1), GPoint(bounds.size.w-1, bounds.size.h-2));
  graphics_draw_line(ctx, GPoint(bounds.size.w-2, 1), GPoint(bounds.size.w-2, bounds.size.h-2));

  // Find the width of the bar
  int width = (int)(float)(((float)s_battery_level / 100.0F) * bounds.size.w-3);

  // Draw the bar
  graphics_context_set_fill_color(ctx, GColorBlueMoon);
  graphics_fill_rect(ctx, GRect(1, 1, width-3, bounds.size.h-2), 0, GCornerNone);
}

static void bt_update_proc(Layer *layer, GContext *ctx) {
 
  bool connected = connection_service_peek_pebble_app_connection();
  //GRect bounds = layer_get_bounds(layer);
  if(connected){
    graphics_context_set_stroke_color(ctx, GColorWhite);
  }
  else {
    graphics_context_set_stroke_color(ctx, GColorRed);
  }
    
  graphics_draw_line(ctx, GPoint(10, 1), GPoint(10, 22));
  graphics_draw_line(ctx, GPoint(11, 1), GPoint(11, 22));
  graphics_draw_line(ctx, GPoint(11, 1), GPoint(16, 6));
  graphics_draw_line(ctx, GPoint(11, 2), GPoint(16, 7));
  graphics_draw_line(ctx, GPoint(16, 6), GPoint(5, 16));
  graphics_draw_line(ctx, GPoint(16, 7), GPoint(5, 17));
  graphics_draw_line(ctx, GPoint(10, 22), GPoint(16, 17));
  graphics_draw_line(ctx, GPoint(11, 22), GPoint(16, 16));
  graphics_draw_line(ctx, GPoint(16, 16), GPoint(5, 6));
  graphics_draw_line(ctx, GPoint(16, 17), GPoint(5, 7));  
  
}

static void progress_layer_update_proc(Layer *layer, GContext *ctx) {
  const GRect inset = grect_inset(layer_get_bounds(layer), GEdgeInsets(2));
  int stepprogress = (int)(float)(((float)s_step_count / s_step_goal) * 360); 

  graphics_context_set_fill_color(ctx, GColorJaegerGreen);
  graphics_fill_radial(ctx, inset, GOvalScaleModeFitCircle, 12,
    DEG_TO_TRIGANGLE(0),
  DEG_TO_TRIGANGLE(stepprogress));
//  DEG_TO_TRIGANGLE((int)(float)(((float)s_step_count / s_step_goal) * 360)));
  graphics_context_set_fill_color(ctx, GColorPictonBlue);
  graphics_fill_radial(ctx, inset, GOvalScaleModeFitCircle, 12,
    DEG_TO_TRIGANGLE(0),
  DEG_TO_TRIGANGLE(stepprogress-360));
}

static void average_layer_update_proc(Layer *layer, GContext *ctx) {
  if(s_step_average < 1) {
    return;
  }

  const GRect inset = grect_inset(layer_get_bounds(layer), GEdgeInsets(2));
  graphics_context_set_fill_color(ctx, GColorYellow);

  int trigangle = DEG_TO_TRIGANGLE(360 * s_step_average / s_step_goal);
  int line_width_trigangle = 1000;
  // draw a very narrow radial (it's just a line)
  graphics_fill_radial(ctx, inset, GOvalScaleModeFitCircle, 12,
    trigangle - line_width_trigangle, trigangle);
}

static void window_load(Window *window) {
  GRect window_bounds = layer_get_bounds(s_window_layer);

  // Dots for the progress indicator
  s_dots_layer = layer_create(window_bounds);
  layer_set_update_proc(s_dots_layer, dots_layer_update_proc);
  layer_add_child(s_window_layer, s_dots_layer);

  // Progress indicator
  s_progress_layer = layer_create(window_bounds);
  layer_set_update_proc(s_progress_layer, progress_layer_update_proc);
  layer_add_child(s_window_layer, s_progress_layer);

  // Average indicator
  s_average_layer = layer_create(window_bounds);
  layer_set_update_proc(s_average_layer, average_layer_update_proc);
  layer_add_child(s_window_layer, s_average_layer);

  // Create a layer to hold the current time
  s_time_layer = text_layer_create(GRect(0, 58, window_bounds.size.w, 45));
  text_layer_set_text_color(s_time_layer, GColorWhite);
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  layer_add_child(s_window_layer, text_layer_get_layer(s_time_layer));

  // Create a layer to hold the current step count
  s_step_layer = text_layer_create(GRect(0, 40, window_bounds.size.w, 38));
  text_layer_set_background_color(s_step_layer, GColorClear);
  text_layer_set_font(s_step_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(s_step_layer, GTextAlignmentCenter);
  layer_add_child(s_window_layer, text_layer_get_layer(s_step_layer));
  
    // Create the TextLayer with specific bounds
  s_date_layer = text_layer_create(GRect(0, 95, window_bounds.size.w, 65));
  // Apply to TextLayer
  text_layer_set_font(s_date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  // Improve the layout to be more like a watchface
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_color(s_date_layer, GColorWhite);
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  // Add it as a child layer to the Window's root layer
  layer_add_child(s_window_layer, text_layer_get_layer(s_date_layer));
  
    // Create the TextLayer with specific bounds
  s_status_layer = text_layer_create(GRect(85, 0, 59, 35));
  // Apply to TextLayer
  text_layer_set_font(s_status_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  // Improve the layout to be more like a watchface
  text_layer_set_background_color(s_status_layer, GColorClear);
  text_layer_set_text_color(s_status_layer, GColorWhite);
  text_layer_set_text_alignment(s_status_layer, GTextAlignmentRight);
  // Add it as a child layer to the Window's root layer
  layer_add_child(s_window_layer, text_layer_get_layer(s_status_layer));
  
  s_bt_layer = layer_create(GRect(120,134,21,23));
  layer_set_update_proc(s_bt_layer, bt_update_proc);
  layer_add_child(s_window_layer, s_bt_layer);
  
    // Create battery meter Layer
  s_battery_layer = layer_create(GRect(3, 144, 25, 9));
  layer_set_update_proc(s_battery_layer, battery_update_proc);
  // Add to Window
  layer_add_child(s_window_layer, s_battery_layer);
}

static void window_unload(Window *window) {
  layer_destroy(text_layer_get_layer(s_time_layer));
  layer_destroy(text_layer_get_layer(s_step_layer));
  layer_destroy(text_layer_get_layer(s_date_layer));
  layer_destroy(text_layer_get_layer(s_status_layer));
  layer_destroy(s_bt_layer);
  layer_destroy(s_battery_layer);
  layer_destroy(s_dots_layer);
  layer_destroy(s_progress_layer);
  layer_destroy(s_average_layer);
  layer_destroy(s_window_layer);
  events_battery_state_service_unsubscribe(battery_callback);
  events_connection_service_unsubscribe(bluetooth_callback);
  events_tick_timer_service_unsubscribe(tick_handler);
  events_app_message_unsubscribe(inbox_received_callback);
  events_app_message_unsubscribe(inbox_dropped_callback);
  events_app_message_unsubscribe(outbox_failed_callback);
  events_app_message_unsubscribe(outbox_sent_callback);
//  window_destroy(window);
}

void init() {
  s_window = window_create();
  s_window_layer = window_get_root_layer(s_window);
  window_set_background_color(s_window, GColorBlack);

  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload
  });
  //generic_weather_init();

  read_persist();
  window_stack_push(s_window, true);

  // Register with TickTimerService
  events_tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  // Make sure the time is displayed from the start
  //tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
  // Register for battery level updates
  events_battery_state_service_subscribe(battery_callback);
  // Ensure battery level is displayed from the start
  battery_callback(battery_state_service_peek());
  
  // Register for Bluetooth connection updates
  events_connection_service_subscribe((ConnectionHandlers) {.pebble_app_connection_handler = bluetooth_callback});

  update_time();
  clear_status();
	events_app_message_request_inbox_size(1024);
	events_app_message_register_inbox_received(inbox_received_callback, NULL);
	events_app_message_register_inbox_dropped(inbox_dropped_callback, NULL);
	events_app_message_register_outbox_failed(outbox_failed_callback, NULL);
	events_app_message_register_outbox_sent(outbox_sent_callback, NULL);
  events_app_message_open();
}

void deinit() {
	store_persist();
//  generic_weather_deinit();
}

int main() {
  init();
  app_event_loop();
  deinit();
}