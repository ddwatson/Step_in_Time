#include <pebble.h>
#include <pebble-events/pebble-events.h>
#include <pebble-generic-weather/pebble-generic-weather.h>

static Window *s_window;
static Layer *s_window_layer, *s_dots_layer, *s_progress_layer, *s_average_layer, *s_battery_layer, *s_bt_layer;
static TextLayer *s_time_layer, *s_step_layer, *s_date_layer, *s_status_layer,  *s_temperature_layer, *icon_weather_layer;
static char s_current_time_buffer[8], s_current_steps_buffer[16], api_key[50], userweatherprovider[7];
static int s_step_count = 1, s_step_count_prev =1, s_step_goal = 1, s_step_average = 1, s_battery_level, s_battery_charging, UserMidStepGoal = 4500, UserStepGoal = 7500, TIMER_INTERVAL_MS=180000, TIMER_IDLE_INTERVAL=46, Current_Min=61, Hibernate_Min=61;;
static char generic_status[]="Day Avg:\n10,000";
static bool F_Tick = S_TRUE, WeatherSetupStatusKey = S_FALSE, WeatherSetupStatusProvider = S_FALSE, WeatherReadyRecieved = S_FALSE, UserSetpGoalType = S_TRUE, WeatherEnabled = S_FALSE, StepGoalEnabled = S_TRUE, HibernateEnable = S_TRUE, SleepEnable = S_TRUE, Watchface_Hibernate = S_FALSE, Watchface_Sleep = S_FALSE;
static int text_color_value =0;
GColor text_color;
static GFont s_weather_icon_font;
static EventHandle s_health_event_handle, s_tick_timer_event_handle;//, s_idle_timer_event_handle;


static void read_persist()
{
	if(persist_exists(MESSAGE_KEY_STEPGOAL))
	{
		UserStepGoal = persist_read_int(MESSAGE_KEY_STEPGOAL);
	}
	if(persist_exists(MESSAGE_KEY_STEPMIDGOAL))
	{
		UserMidStepGoal=persist_read_int(MESSAGE_KEY_STEPMIDGOAL);
	}
	if(persist_exists(MESSAGE_KEY_STEPGOALENABLED))
	{
		StepGoalEnabled = persist_read_bool(MESSAGE_KEY_STEPGOALENABLED);
	}
	if(persist_exists(MESSAGE_KEY_SLEEPENABLED)) {
		SleepEnable = persist_read_bool(MESSAGE_KEY_SLEEPENABLED);
	}
	if(persist_exists(MESSAGE_KEY_HIBERNATEENABLED)) {
		HibernateEnable = persist_read_bool(MESSAGE_KEY_HIBERNATEENABLED);
	}
  if(persist_exists(MESSAGE_KEY_GOALTYPE)) {
    UserSetpGoalType = persist_read_bool(MESSAGE_KEY_GOALTYPE);
  }
  if(persist_exists(MESSAGE_KEY_APIKEY)) 	{
		persist_read_string(MESSAGE_KEY_APIKEY, api_key, sizeof(api_key));
	}
	if(persist_exists(MESSAGE_KEY_FTICK)) {
		F_Tick = persist_read_bool(MESSAGE_KEY_FTICK);
	}
	if(persist_exists(MESSAGE_KEY_WeatherProvide)) {
		persist_read_string(MESSAGE_KEY_WeatherProvide, userweatherprovider, sizeof(userweatherprovider));
    if (strlen(userweatherprovider)>1) {WeatherEnabled=S_TRUE;}
	}
}

static void store_persist()
{
  persist_write_int(MESSAGE_KEY_STEPGOAL, UserStepGoal);
  persist_write_int(MESSAGE_KEY_STEPMIDGOAL, UserMidStepGoal);
  persist_write_bool(MESSAGE_KEY_STEPGOALENABLED, StepGoalEnabled);
  persist_write_bool(MESSAGE_KEY_GOALTYPE, UserSetpGoalType);
  persist_write_bool(MESSAGE_KEY_SLEEPENABLED, SleepEnable);
  persist_write_bool(MESSAGE_KEY_HIBERNATEENABLED, HibernateEnable);
	persist_write_string(MESSAGE_KEY_APIKEY, api_key);
  persist_write_bool(MESSAGE_KEY_FTICK, F_Tick);
	persist_write_string(MESSAGE_KEY_WeatherProvide, userweatherprovider);
  if (text_color_value >0) { persist_write_int(MESSAGE_KEY_TEXTCOLOR, text_color_value); }
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

static void weather_callback(GenericWeatherInfo *info, GenericWeatherStatus status) {
if (WeatherEnabled){
  static char s_buffer[16];
  switch(status) {
    case GenericWeatherStatusAvailable:
    {
        clear_status();  //clears out any previous messages
//      static char s_city_buffer[32];
//      static char s_weather_description[14];
//      snprintf(s_buffer, sizeof(s_buffer),"Temperature (K/C/F): %d/%d/%d\n\nName:\n%s\n\nDescription:\n%s",info->temp_k, info->temp_c, info->temp_f, info->name, info->description);
      if (F_Tick) {
        snprintf(s_buffer, sizeof(s_buffer),"%d˚", info->temp_f);}
      else {
        snprintf(s_buffer, sizeof(s_buffer),"%d˚", info->temp_c);}
      text_layer_set_text(s_temperature_layer, s_buffer);
//      snprintf(s_city_buffer, sizeof(s_city_buffer),"%s",info->name);
//      text_layer_set_text(s_city_layer, s_city_buffer);
//      if (WeatherDescriptionDisp) {
//        snprintf(s_weather_description, sizeof(s_weather_description),"%s",info->description);
//        text_layer_set_text(s_weatherdescript_layer, s_weather_description);
//      }		
//      else {text_layer_set_text(s_weatherdescript_layer, " ");}
		switch(info->condition)
			{
				case GenericWeatherConditionClearSky:
					if(info->day)
					{
						text_layer_set_text(icon_weather_layer, "J");
					}
					else
					{
						text_layer_set_text(icon_weather_layer, "D");
					}
					break;
				case GenericWeatherConditionFewClouds:
					if(info->day)
					{
						text_layer_set_text(icon_weather_layer, "F");
					}
					else
					{
						text_layer_set_text(icon_weather_layer, "E");
					}
					break;
				case GenericWeatherConditionScatteredClouds:
					text_layer_set_text(icon_weather_layer, "A");
					break;
				case GenericWeatherConditionBrokenClouds:
					text_layer_set_text(icon_weather_layer, "A");
					break;
				case GenericWeatherConditionShowerRain:
					text_layer_set_text(icon_weather_layer, "G");
					break;
				case GenericWeatherConditionRain:
					text_layer_set_text(icon_weather_layer, "G");
					break;
				case GenericWeatherConditionThunderstorm:
					text_layer_set_text(icon_weather_layer, "I");
					break;
				case GenericWeatherConditionSnow:
					text_layer_set_text(icon_weather_layer, "H");
					break;
				case GenericWeatherConditionMist:
					text_layer_set_text(icon_weather_layer, "g");
					break;
				case GenericWeatherConditionUnknown:
					text_layer_set_text(icon_weather_layer, "c");
					break;
			}
    }
      break;
    case GenericWeatherStatusNotYetFetched:
//      text_layer_set_text(s_status_layer, "Load...");
      break;
    case GenericWeatherStatusBluetoothDisconnected:
      text_layer_set_text(s_status_layer, "No BT");
      app_timer_register(TIMER_INTERVAL_MS, clear_status, NULL);
      break;
    case GenericWeatherStatusPending:
//      text_layer_set_text(s_status_layer, "Load...");
      break;
    case GenericWeatherStatusFailed:
      snprintf(s_buffer, sizeof(s_buffer),"%s Failed", userweatherprovider);
      text_layer_set_text(s_status_layer, s_buffer);
      app_timer_register(TIMER_INTERVAL_MS, clear_status, NULL);
      break;
    case GenericWeatherStatusBadKey:
      snprintf(s_buffer, sizeof(s_buffer),"%s Bad Key", userweatherprovider);
      text_layer_set_text(s_status_layer, s_buffer);
      app_timer_register(TIMER_INTERVAL_MS, clear_status, NULL);
      break;
    case GenericWeatherStatusLocationUnavailable:
      text_layer_set_text(s_status_layer, "No Locate");
      app_timer_register(TIMER_INTERVAL_MS, clear_status, NULL);
      break;
  }}
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
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Outbox send success!");
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

  if(thousands > 0) {
    snprintf(s_current_steps_buffer, sizeof(s_current_steps_buffer),
      "%d,%03d steps", thousands, hundreds);
  } else {
    snprintf(s_current_steps_buffer, sizeof(s_current_steps_buffer),
      "%d steps", hundreds);
  }
  text_layer_set_text(s_step_layer, s_current_steps_buffer);
}

static void update_steps() {
  s_step_count_prev = s_step_count;
  s_step_count = get_health(HealthMetricStepCount, 0);
  s_step_average = get_health(HealthMetricStepCount, 1);
  if (UserSetpGoalType) {
    s_step_goal = get_health(HealthMetricStepCount, 2);
  } else {
    s_step_goal = UserStepGoal;
  }
  if(s_step_goal==0){s_step_goal = UserStepGoal;}  //just use the user's step goal if we have no goal data
  if (s_step_average==0) {//this comes up when the watch has less than a week of health data
    time_t temp = time(NULL);
    struct tm *tick_time = localtime(&temp);
    int current_hour = tick_time->tm_hour;
    if (current_hour<6) {
      s_step_average = 100;
    } else if (current_hour>21) {
      s_step_average = s_step_goal;
    } else {
      s_step_average=(((current_hour-5)*s_step_goal)/16);  //calculate average by the time of day excluding the time before 
    }
  }
  if (s_step_count_prev <2) {s_step_count_prev = s_step_count;}
  display_step_count();
  layer_mark_dirty(s_progress_layer);
  layer_mark_dirty(s_average_layer);
  if (StepGoalEnabled&&(s_step_count_prev<UserStepGoal)&&(s_step_count>=UserStepGoal)) {
    static const uint32_t const segments[] = { 200, 200, 50, 50, 50, 50, 100, 200, 400 };
    VibePattern pat = {
      .durations = segments,
      .num_segments = ARRAY_LENGTH(segments),
    };
    vibes_enqueue_custom_pattern(pat);
    text_layer_set_text(s_status_layer, "Day Goal\nMet");
    app_timer_register(TIMER_INTERVAL_MS, clear_status, NULL);
  }
  else if (StepGoalEnabled&&(s_step_count_prev<UserMidStepGoal)&&(s_step_count>=UserMidStepGoal)) {
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

static void update_time() {
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  Current_Min=tick_time->tm_min;
  strftime(s_current_time_buffer, sizeof(s_current_time_buffer),
           clock_is_24h_style() ? "%H:%M" : "%l:%M", tick_time);
  text_layer_set_text(s_time_layer, s_current_time_buffer);
  static char s_weekdayname[30];
  strftime(s_weekdayname, sizeof(s_weekdayname), "%A%n%b %e", tick_time);
  text_layer_set_text(s_date_layer, s_weekdayname);
}

static void watchface_refresh(){

  update_steps();
  update_time();
  clear_status();
  if (WeatherEnabled) {generic_weather_fetch(weather_callback);} 
}

static void accel_tap_handler(AccelAxisType axis, int32_t direction) {
  // A tap event occured
  Watchface_Hibernate=S_FALSE;
  Watchface_Sleep = S_FALSE;
  // Unsubscribe from tap events
  accel_tap_service_unsubscribe();
  watchface_refresh();
  if (HibernateEnable) {
    Hibernate_Min = (Current_Min + TIMER_IDLE_INTERVAL) % 60;  //I tried using a timer above but the interval is not set correctly on first run and the user has to relaunch the watchface.  Instead I am building my own timer
  } else {
    Hibernate_Min =61;  //this will never be the current minute so this disables the idle timer
  }}

static void tick_handler(struct tm *tick_time, TimeUnits changed) {
  if (!Watchface_Hibernate && !Watchface_Sleep) {  //if we are no sleeping or hibernating
    update_steps();
    update_time();
    if ((tick_time->tm_min % 30 == 0)&&WeatherEnabled) {  
      generic_weather_fetch(weather_callback);
    }
     if ((tick_time->tm_min == Hibernate_Min)&&HibernateEnable&&!Watchface_Sleep) {
      Watchface_Hibernate =S_TRUE;
      text_layer_set_text(s_status_layer,"Watch is Idle");
      text_layer_set_text(s_time_layer, "...");
      accel_tap_service_subscribe(accel_tap_handler);
    }
  }
}

bool is_user_sleeping() {
  static bool is_sleeping=S_FALSE;
  if (SleepEnable) {
    HealthActivityMask activities = health_service_peek_current_activities();
    is_sleeping = activities & HealthActivitySleep || activities & HealthActivityRestfulSleep;
  }
  return is_sleeping;
/*  if (s_tick_timer_event_handle!=NULL) {
    return S_TRUE;
  } else {
    return S_FALSE;
  }*/
}

static void prv_health_event_handler(HealthEventType event, void *context) {
    switch(event) {
    case HealthEventSignificantUpdate: 
      if (!Watchface_Hibernate && HibernateEnable) {
        Hibernate_Min = (Current_Min + TIMER_IDLE_INTERVAL) % 60;
      }
      prv_health_event_handler(HealthEventSleepUpdate, context);
      break;
    case HealthEventSleepUpdate: {
      if (SleepEnable) {
        bool sleeping = is_user_sleeping();
        if (sleeping && !Watchface_Sleep) {
          Watchface_Sleep = S_TRUE;
          text_layer_set_text(s_status_layer,"Watch is Sleeping");
          text_layer_set_text(s_time_layer, "...");
          Watchface_Hibernate = S_FALSE;  //if the user falls asleep while in hibernation then reset to wake when the user wakes
          accel_tap_service_subscribe(accel_tap_handler);
        } else if (!sleeping && !Watchface_Hibernate) {
          accel_tap_service_unsubscribe();  //do this here because having a Null tick_timer coorelates with subscribing to tap service
          Watchface_Sleep = S_FALSE;
          Watchface_Hibernate = S_FALSE;
          watchface_refresh();
          if (HibernateEnable) {
            Hibernate_Min = (Current_Min + TIMER_IDLE_INTERVAL) % 60;
          } else {
            Hibernate_Min =61;
          }
        }
      }
    }
      break;
    case HealthEventMovementUpdate:
      if (Watchface_Hibernate||Watchface_Sleep) {
        Watchface_Sleep = S_FALSE;
        Watchface_Hibernate = S_FALSE;
        watchface_refresh();
        accel_tap_service_unsubscribe();  //do this here because having a Null tick_timer coorelates with subscribing to tap service
      }
      if (HibernateEnable&&!Watchface_Hibernate) {Hibernate_Min = (Current_Min + TIMER_IDLE_INTERVAL) % 60;}
      break;
    case HealthEventHeartRateUpdate:
//      app_timer_reschedule(s_idle_timer_event_handle, TIMER_IDLE_INTERVAL_MS);
      break;
    case HealthEventMetricAlert:
      break;
    }
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
  int width = (s_battery_level * (bounds.size.w-2))/100;

  // Draw the bar
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, GRect(1, 1, width, bounds.size.h-2), 0, GCornerNone);
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
  int stepprogress = (s_step_count*360) / s_step_goal;

  graphics_context_set_fill_color(ctx, GColorJaegerGreen);
  graphics_fill_radial(ctx, inset, GOvalScaleModeFitCircle, 12,DEG_TO_TRIGANGLE(0),DEG_TO_TRIGANGLE(stepprogress));
  graphics_context_set_fill_color(ctx, GColorPictonBlue);
  graphics_fill_radial(ctx, inset, GOvalScaleModeFitCircle, 12,DEG_TO_TRIGANGLE(0),DEG_TO_TRIGANGLE(stepprogress-360));
  graphics_context_set_fill_color(ctx, GColorRed);
  graphics_fill_radial(ctx, inset, GOvalScaleModeFitCircle, 12,DEG_TO_TRIGANGLE(0),DEG_TO_TRIGANGLE(stepprogress-720));
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

static void WeatherInitDeinit() {
  if ((strlen(userweatherprovider)>0) && (strlen(api_key)>0)) {
    //APP_LOG(APP_LOG_LEVEL_DEBUG, "WeatherInit");
    if (WeatherSetupStatusProvider==S_FALSE) {generic_weather_init();}
    generic_weather_set_api_key(api_key);
    WeatherSetupStatusKey=S_TRUE;
    if (strcmp(userweatherprovider,"OpenWe")==0)
      {generic_weather_set_provider(GenericWeatherProviderOpenWeatherMap);
      WeatherSetupStatusProvider=S_TRUE;}
    else if(strcmp(userweatherprovider,"WUnder")==0)
      {generic_weather_set_provider(GenericWeatherProviderWeatherUnderground);
      WeatherSetupStatusProvider=S_TRUE;}
    else if(strcmp(userweatherprovider,"For.io")==0)
      {generic_weather_set_provider(GenericWeatherProviderForecastIo);
      WeatherSetupStatusProvider=S_TRUE;}
    else
      {APP_LOG(APP_LOG_LEVEL_DEBUG, "UNKNOWN PROVIDER: -%s-", userweatherprovider);}
  } else if (WeatherSetupStatusProvider==S_TRUE) {
    generic_weather_deinit();
    WeatherSetupStatusProvider=S_FALSE;
    WeatherReadyRecieved = S_FALSE;
    WeatherSetupStatusKey=S_FALSE;
    //APP_LOG(APP_LOG_LEVEL_DEBUG, "WeatherDeinit");
  }
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context)
{
	Tuple *data = dict_find(iterator, MESSAGE_KEY_READY);
	if(data) 	{
    WeatherReadyRecieved = S_TRUE;
		//APP_LOG(APP_LOG_LEVEL_DEBUG, "Ready Received!. Requesting Weather");
//    snprintf(generic_status, sizeof(generic_status), "!%d!%d!%d", WeatherSetupStatusKey, WeatherSetupStatusProvider, WeatherEnabled);
//    text_layer_set_text(s_status_layer, generic_status);
    if(WeatherSetupStatusKey&&WeatherSetupStatusProvider&&WeatherEnabled) {
      generic_weather_fetch(weather_callback);
    }
  }
  
	data = dict_find(iterator, MESSAGE_KEY_STEPGOAL);
	if(data) 	{
		UserStepGoal = data->value->uint32;
	}

	data = dict_find(iterator, MESSAGE_KEY_STEPMIDGOAL);
	if(data) 	{
		UserMidStepGoal= data->value->uint32;
	}
  
	data = dict_find(iterator, MESSAGE_KEY_STEPGOALENABLED);
	if(data) 	{
		StepGoalEnabled = data->value->int32 == 1;
	}
  
	data = dict_find(iterator, MESSAGE_KEY_SLEEPENABLED);
	if(data)
	{
		SleepEnable = data->value->int32 == 1;
	}

	data = dict_find(iterator, MESSAGE_KEY_HIBERNATEENABLED);
	if(data)
	{
		HibernateEnable = data->value->int32 == 1;

    events_health_service_events_unsubscribe(s_health_event_handle);
    //register to recieve significant updates of which we are looking for sleep updates
    if (SleepEnable||HibernateEnable) {s_health_event_handle = events_health_service_events_subscribe(prv_health_event_handler, NULL);}
	}
  
	data = dict_find(iterator, MESSAGE_KEY_TEXTCOLOR);
  if(data) {
    if (data->value->int32!=text_color_value) {
      text_color_value = data->value->int32;
      text_color = GColorFromHEX(text_color_value);
      text_layer_set_text_color(s_time_layer, text_color);
      text_layer_set_text_color(s_date_layer, text_color);
      text_layer_set_text_color(icon_weather_layer, text_color);
      text_layer_set_text_color(s_temperature_layer, text_color);
      text_layer_set_text_color(s_date_layer, text_color);
      text_layer_set_text_color(s_status_layer, text_color);
    }
  }  
  
	data = dict_find(iterator, MESSAGE_KEY_APIKEY);
	if(data)
	{
		strcpy(api_key, data->value->cstring);
    if (strlen(api_key)>0) {
      generic_weather_set_api_key(api_key);
      WeatherSetupStatusKey=S_TRUE;
    } else {
      WeatherSetupStatusKey=S_FALSE;
    }
	}

	data = dict_find(iterator, MESSAGE_KEY_FTICK);
	if(data)
	{
		F_Tick = data->value->int32 == 1;
	}

  data = dict_find(iterator, MESSAGE_KEY_WeatherProvide);
	if(data)
	{
    strcpy(userweatherprovider, data->value->cstring);
    WeatherInitDeinit();
    WeatherEnabled = WeatherSetupStatusProvider && WeatherSetupStatusKey;
  }

  data = dict_find(iterator, MESSAGE_KEY_GOALTYPE);
  if(data) {
    UserSetpGoalType = data->value->int32 == 1;
    //this is the last setting so go ahead and adjust the display to the new settings
    update_steps();
    layer_mark_dirty(s_progress_layer);
    layer_mark_dirty(s_average_layer);
    if(WeatherSetupStatusKey&&WeatherSetupStatusProvider&&WeatherEnabled&&WeatherReadyRecieved) {
      generic_weather_fetch(weather_callback);
    } else {
      text_layer_set_text(icon_weather_layer, "M");  //this will blank the weather icon
      text_layer_set_text(s_temperature_layer, " ");
    }
  }
}

static void window_load(Window *window) {
  GRect window_bounds = layer_get_bounds(s_window_layer);
  s_weather_icon_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_WEATHERICON_38));

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
  text_layer_set_text_color(s_time_layer, text_color);
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  layer_add_child(s_window_layer, text_layer_get_layer(s_time_layer));

  // Create a layer to hold the current step count
  s_step_layer = text_layer_create(GRect(0, 40, window_bounds.size.w, 38));
  text_layer_set_text_color(s_step_layer, GColorPictonBlue);
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
  text_layer_set_text_color(s_date_layer, text_color);
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  // Add it as a child layer to the Window's root layer
  layer_add_child(s_window_layer, text_layer_get_layer(s_date_layer));
  
    // Create the TextLayer with specific bounds
  s_status_layer = text_layer_create(GRect(85, 0, 59, 35));
  // Apply to TextLayer
  text_layer_set_font(s_status_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  // Improve the layout to be more like a watchface
  text_layer_set_background_color(s_status_layer, GColorClear);
  text_layer_set_text_color(s_status_layer, text_color);
  text_layer_set_text_alignment(s_status_layer, GTextAlignmentRight);
  // Add it as a child layer to the Window's root layer
  layer_add_child(s_window_layer, text_layer_get_layer(s_status_layer));
  
  //layer for the weather icon
  icon_weather_layer = text_layer_create(GRect(0, 12, window_bounds.size.w, 40));
  // Apply to TextLayer
  text_layer_set_font(icon_weather_layer, s_weather_icon_font);
  // Improve the layout to be more like a watchface
  text_layer_set_background_color(icon_weather_layer, GColorClear);
  text_layer_set_text_color(icon_weather_layer, text_color);
  text_layer_set_text_alignment(icon_weather_layer, GTextAlignmentCenter);
  // Add it as a child layer to the Window's root layer
  layer_add_child(s_window_layer, text_layer_get_layer(icon_weather_layer));
  
  //layer for the tempature
  s_temperature_layer = text_layer_create(GRect(0, 0, 40, 42));
  text_layer_set_text_color(s_temperature_layer, text_color);
  text_layer_set_background_color(s_temperature_layer, GColorClear);
  text_layer_set_font(s_temperature_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text_alignment(s_temperature_layer, GTextAlignmentLeft);
  layer_add_child(s_window_layer, text_layer_get_layer(s_temperature_layer));
  
  //layer for the bluetooth icon
  s_bt_layer = layer_create(GRect(120,134,21,23));
  layer_set_update_proc(s_bt_layer, bt_update_proc);
  layer_add_child(s_window_layer, s_bt_layer);
  
    // Create battery meter Layer
  s_battery_layer = layer_create(GRect(0, 144, 28, 9));
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
  text_layer_destroy(s_temperature_layer);
  text_layer_destroy(icon_weather_layer);

    // Unload GFont
  fonts_unload_custom_font(s_weather_icon_font);

  //unregister from services
  events_battery_state_service_unsubscribe(battery_callback);
  events_connection_service_unsubscribe(bluetooth_callback);
  events_tick_timer_service_unsubscribe(tick_handler);
  events_app_message_unsubscribe(inbox_received_callback);
  events_app_message_unsubscribe(inbox_dropped_callback);
  events_app_message_unsubscribe(outbox_failed_callback);
  events_app_message_unsubscribe(outbox_sent_callback);

  //destroy root window
  window_destroy(window);
}

void init() {
	//I shouldn't be doing this here and it should be in read_persist, but the function takes a while to complete and I need this setting for window_load
  text_color = persist_exists(MESSAGE_KEY_TEXTCOLOR)? GColorFromHEX(persist_read_int(MESSAGE_KEY_TEXTCOLOR)) : GColorWhite;

  s_window = window_create();
  s_window_layer = window_get_root_layer(s_window);
  window_set_background_color(s_window, GColorBlack);

  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload
  });

  read_persist();
  window_stack_push(s_window, true);

  //initalize weather
  generic_weather_init();
  if (strlen(api_key)>0) {
    WeatherSetupStatusKey=S_TRUE;
    generic_weather_set_api_key(api_key);
  }
  if (strcmp(userweatherprovider,"OpenWe")==0)
    {generic_weather_set_provider(GenericWeatherProviderOpenWeatherMap);
    WeatherSetupStatusProvider=S_TRUE;}
  else if(strcmp(userweatherprovider,"WUnder")==0)
    {generic_weather_set_provider(GenericWeatherProviderWeatherUnderground);
    WeatherSetupStatusProvider=S_TRUE;}
	else if(strcmp(userweatherprovider,"For.io")==0)
    {generic_weather_set_provider(GenericWeatherProviderForecastIo);
    WeatherSetupStatusProvider=S_TRUE;}
  else
    { WeatherEnabled = S_FALSE;
    }

  // Register with TickTimerService
  s_tick_timer_event_handle = events_tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
  //register to recieve significant updates of which we are looking for sleep updates
  if (SleepEnable||HibernateEnable) {s_health_event_handle = events_health_service_events_subscribe(prv_health_event_handler, NULL);}
  
  //register to detect when the watch goes idle
  if (HibernateEnable) {Hibernate_Min = (Current_Min + TIMER_IDLE_INTERVAL) % 60;}
  
  // Register for battery level updates
  events_battery_state_service_subscribe(battery_callback);
  // Ensure battery level is displayed from the start
  battery_callback(battery_state_service_peek());
  
  // Register for Bluetooth connection updates
  events_connection_service_subscribe((ConnectionHandlers) {.pebble_app_connection_handler = bluetooth_callback});

  // Make sure the time, steps, and status are displayed from the start
  update_steps();
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
  generic_weather_deinit();
}

int main() {
  init();
  app_event_loop();
  deinit();
}