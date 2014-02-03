#include <pebble.h>

Window *window;
Layer *clock_layer, *secs_layer;
TextLayer *ddmm_layer, *hhmm_layer, *ss_layer;
BitmapLayer *background_layer, *battery_layer, *radio_layer, *dst_layer;
InverterLayer *inv_layer;

static GBitmap *background, *batteryAll;
static GFont digitS, digitM, digitL;

static GPath *hour_arrow, *minute_arrow, *second_arrow;

char ddmmBuffer[] = "00-00";
char hhmmBuffer[] = "00:00";
char ssBuffer[] = "00";

static const GPathInfo HOUR_HAND_POINTS =
{
	11, (GPoint []) { { 0, -8 }, { -3, -30 }, { -2, -30 }, { 0, -8 }, { -1, -30 }, { 0, -30 }, { 0, -8 }, { 1, -30 }, { 2, -30 }, { 0, -8 }, { -3, -30 } } 
};


static const GPathInfo MINUTE_HAND_POINTS =
{
	14, (GPoint []) { { -3, -33 }, { -3, -39 }, { -2, -39 }, { -2, -33 }, { -1, -33 }, { -1, -39 }, { 0, -39 }, { 0, -33 }, { 1, -33 }, { 1, -39 }, { 2, -39 }, { 2, -33 }, { 3, -33 }, { 3, -39 } } 
};

static const GPathInfo SECOND_HAND_POINTS =
{
	3, (GPoint []) { { 0, -7 }, { -1, -26 }, { 1, -26 } } 
};

/*
void battery_state_service_handler(BatteryChargeState charge_state) 
{
	int nImage = 0;
	if (charge_state.is_charging)
		nImage = 10;
	else 
		nImage = 10 - (charge_state.charge_percent / 10);
	
	GRect sub_rect = GRect(0, 10*nImage, 20, 10*nImage+10);
	bitmap_layer_set_bitmap(battery_layer, gbitmap_create_as_sub_bitmap(batteryAll, sub_rect));
}
*/
/*
void bluetooth_connection_handler(bool connected)
{
	layer_set_hidden(bitmap_layer_get_layer(radio_layer), connected != true);
}
*/

static void clock_update_proc(Layer *layer, GContext *ctx) 
{
	GRect bounds = layer_get_bounds(layer);
	const GPoint center = grect_center_point(&bounds);
	graphics_context_set_stroke_color(ctx, GColorBlack);
	graphics_context_set_fill_color(ctx, GColorBlack);
	
	time_t now = time(NULL);
	struct tm *t = localtime(&now);

	gpath_move_to(hour_arrow, center);
	gpath_move_to(minute_arrow, center);

	//Hour Arrow
	gpath_rotate_to(hour_arrow, (TRIG_MAX_ANGLE * (((t->tm_hour % 12) * 6) + (t->tm_min / 10))) / (12 * 6));
	//gpath_draw_filled(ctx, hour_arrow); //Filling don't work on small objects :(
	gpath_draw_outline(ctx, hour_arrow);
		
	//Minute = Hour Arrow + Minute Arrow
	gpath_rotate_to(hour_arrow, TRIG_MAX_ANGLE * t->tm_min / 60);
	//gpath_draw_filled(ctx, hour_arrow);
	gpath_draw_outline(ctx, hour_arrow);
	gpath_rotate_to(minute_arrow, TRIG_MAX_ANGLE * t->tm_min / 60);
	//gpath_draw_filled(ctx, minute_arrow);
	gpath_draw_outline(ctx, minute_arrow);
}

static void secs_update_proc(Layer *layer, GContext *ctx) 
{
	GRect bounds = layer_get_bounds(layer);
	const GPoint center = grect_center_point(&bounds);
	graphics_context_set_stroke_color(ctx, GColorBlack);
	graphics_context_set_fill_color(ctx, GColorBlack);
	
	time_t now = time(NULL);
	struct tm *t = localtime(&now);

	gpath_move_to(second_arrow, center);
	
	for (int32_t i=0; i<=t->tm_sec; i++)
	{
		gpath_rotate_to(second_arrow, TRIG_MAX_ANGLE * i / 60);
		gpath_draw_filled(ctx, second_arrow);
		gpath_draw_outline(ctx, second_arrow);
	}	
/*	
	GPoint ptSecE, ptSecA;
	const int16_t SecLen = bounds.size.w / 2;

	for (int32_t i=0; i<=t->tm_sec; i++)
	{
		int32_t second_angle = TRIG_MAX_ANGLE * i / 60;
		ptSecE.x = (int16_t)(sin_lookup(second_angle) * (int32_t)SecLen / TRIG_MAX_RATIO) + center.x;
		ptSecE.y = (int16_t)(-cos_lookup(second_angle) * (int32_t)SecLen / TRIG_MAX_RATIO) + center.y;
		ptSecA.x = (int16_t)(sin_lookup(second_angle) * (int32_t)6 / TRIG_MAX_RATIO) + center.x;
		ptSecA.y = (int16_t)(-cos_lookup(second_angle) * (int32_t)6 / TRIG_MAX_RATIO) + center.y;
		graphics_draw_line(ctx, ptSecE, ptSecA);
	}
*/
}

void tick_handler(struct tm *tick_time, TimeUnits units_changed)
{
	int seconds = tick_time->tm_sec;
	layer_mark_dirty(secs_layer);
	if ((seconds >= 0 && seconds <= 3) || MINUTE_UNIT)
		layer_mark_dirty(clock_layer);

/*
	strftime(ssBuffer, sizeof(ssBuffer), "%S", tick_time);
	text_layer_set_text(ss_layer, ssBuffer);

	if(seconds == 0 || units_changed == MINUTE_UNIT)
	{
		if(clock_is_24h_style())
			strftime(hhmmBuffer, sizeof(hhmmBuffer), "%H:%M", tick_time);
		else
			strftime(hhmmBuffer, sizeof(hhmmBuffer), "%I:%M", tick_time);
		
		//strcpy(hhmmBuffer, "22:22");
		text_layer_set_text(hhmm_layer, hhmmBuffer);
		
		strftime(ddmmBuffer, sizeof(ddmmBuffer), "%d-%m", tick_time);
		//snprintf(ddmmBuffer, sizeof(ddmmBuffer), "%d", rc.origin.x);
		text_layer_set_text(ddmm_layer, ddmmBuffer);
		
		strftime(wdBuffer, sizeof(wdBuffer), "%a", tick_time);
		upcase(wdBuffer);
		text_layer_set_text(wd_layer, wdBuffer);

		strftime(yyyyBuffer, sizeof(yyyyBuffer), "%Y", tick_time);
		text_layer_set_text(yyyy_layer, yyyyBuffer);

		//Check DST at 4h at morning
		if ((tick_time->tm_hour == 4 && tick_time->tm_min == 0) || units_changed == MINUTE_UNIT)
			layer_set_hidden(bitmap_layer_get_layer(dst_layer), tick_time->tm_isdst != 1);
	}
*/
}

void window_load(Window *window)
{
	Layer *window_layer = window_get_root_layer(window);

	//Init Background
	background = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND);
	background_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
	bitmap_layer_set_background_color(background_layer, GColorClear);
	bitmap_layer_set_bitmap(background_layer, background);
	layer_add_child(window_layer, bitmap_layer_get_layer(background_layer));
	
	//Init Clock Layer
	clock_layer = layer_create(GRect(5, 26, 79, 79));
	layer_set_update_proc(clock_layer, clock_update_proc);
	hour_arrow = gpath_create(&HOUR_HAND_POINTS);
	minute_arrow = gpath_create(&MINUTE_HAND_POINTS);
	layer_add_child(window_layer, clock_layer);	
	
	//Init Clock Layer
	secs_layer = layer_create(GRect(86, 26, 53, 53));
	layer_set_update_proc(secs_layer, secs_update_proc);
	second_arrow = gpath_create(&SECOND_HAND_POINTS);
	layer_add_child(window_layer, secs_layer);	
	
	digitS = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DIGITAL_20));
	digitM = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DIGITAL_30));
	digitL = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DIGITAL_40));

/*	//DAY+MONTH layer
	ddmm_layer = text_layer_create(GRect(2, 5, 70, 32));
	text_layer_set_background_color(ddmm_layer, GColorClear);
	text_layer_set_text_color(ddmm_layer, GColorBlack);
	text_layer_set_text_alignment(ddmm_layer, GTextAlignmentCenter);
	text_layer_set_font(ddmm_layer, digitS);
	layer_add_child(window_layer, text_layer_get_layer(ddmm_layer));
*/      
/*	//HOUR+MINUTE layer
	hhmm_layer = text_layer_create(GRect(-110, 50, 110, 75));
	text_layer_set_background_color(hhmm_layer, GColorClear);
	text_layer_set_text_color(hhmm_layer, GColorBlack);
	text_layer_set_text_alignment(hhmm_layer, GTextAlignmentCenter);
	text_layer_set_font(hhmm_layer, digitL);
	layer_add_child(window_layer, text_layer_get_layer(hhmm_layer));
*/      
/*	//SECOND layer
	ss_layer = text_layer_create(GRect(111, 55, 30, 30));
	text_layer_set_background_color(ss_layer, GColorClear);
	text_layer_set_text_color(ss_layer, GColorBlack);
	text_layer_set_text_alignment(ss_layer, GTextAlignmentCenter);
	text_layer_set_font(ss_layer, digitS);
	layer_add_child(window_layer, text_layer_get_layer(ss_layer));
*/      
/*	//Init battery
	batteryAll = gbitmap_create_with_resource(RESOURCE_ID_RESOURCE_ID_IMAGE_BATTERIES);
	battery_layer = bitmap_layer_create(GRect(116, 90, 20, 10)); 
	bitmap_layer_set_background_color(battery_layer, GColorClear);
	layer_add_child(window_layer, bitmap_layer_get_layer(battery_layer));
*/
/*	//DST layer, have to be after battery, uses its image
	dst_layer = bitmap_layer_create(GRect(123, 52, 12, 5));
	bitmap_layer_set_background_color(dst_layer, GColorClear);
	bitmap_layer_set_bitmap(dst_layer, gbitmap_create_as_sub_bitmap(batteryAll, GRect(0, 110, 12, 5)));
	layer_add_child(window_layer, bitmap_layer_get_layer(dst_layer));
*/      
/*	//Init bluetooth radio
	radio = gbitmap_create_with_resource(RESOURCE_ID_RESOURCE_ID_IMAGE_RADIO);
	radio_layer = bitmap_layer_create(GRect(106, 130, 31, 33));
	bitmap_layer_set_background_color(radio_layer, GColorClear);
	bitmap_layer_set_bitmap(radio_layer, radio);
	bitmap_layer_set_compositing_mode(radio_layer, GCompOpAnd);
	layer_add_child(window_layer, bitmap_layer_get_layer(radio_layer));
*/	
	//Init inverter_layer
	inv_layer = inverter_layer_create(GRect(0, 0, 144, 168));
	//layer_add_child(window_layer, inverter_layer_get_layer(inv_layer));

	//Get a time structure so that it doesn't start blank
	time_t temp = time(NULL);
	struct tm *t = localtime(&temp);

	//Manually call the tick handler when the window is loading
	tick_handler(t, MINUTE_UNIT);
/*	
	//Set Battery state
	BatteryChargeState btchg = battery_state_service_peek();
	battery_state_service_handler(btchg);
	
	//Set Bluetooth state
	bool connected = bluetooth_connection_service_peek();
	bluetooth_connection_handler(connected);
*/
}
 
void window_unload(Window *window)
{
	//Destroy Layers
	layer_destroy(clock_layer);
	layer_destroy(secs_layer);
	
	//Destroy GPaths
	gpath_destroy(hour_arrow);
	gpath_destroy(minute_arrow);
	gpath_destroy(second_arrow);
	
	//Destroy text layers
//	text_layer_destroy(ddmm_layer);
//	text_layer_destroy(hhmm_layer);
//	text_layer_destroy(ss_layer);
	
	//Unload Fonts
	fonts_unload_custom_font(digitS);
	fonts_unload_custom_font(digitM);
	fonts_unload_custom_font(digitL);
	
	//Destroy GBitmaps
//	gbitmap_destroy(batteryAll);
	gbitmap_destroy(background);

	//Destroy BitmapLayers
//	bitmap_layer_destroy(dst_layer);
//	bitmap_layer_destroy(battery_layer);
//	bitmap_layer_destroy(radio_layer);
	bitmap_layer_destroy(background_layer);
	
	//Destroy Inverter Layer
	inverter_layer_destroy(inv_layer);
}

void handle_init(void) {
	window = window_create();
	window_set_window_handlers(window, (WindowHandlers) {
		.load = window_load,
		.unload = window_unload,
	});
    window_stack_push(window, true);
	tick_timer_service_subscribe(SECOND_UNIT, (TickHandler)tick_handler);
//	battery_state_service_subscribe(&battery_state_service_handler);
//	bluetooth_connection_service_subscribe(&bluetooth_connection_handler);
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);
}

void handle_deinit(void) {
	tick_timer_service_unsubscribe();
//	battery_state_service_unsubscribe();
//	bluetooth_connection_service_unsubscribe();
	window_destroy(window);
}

int main(void) {
	  handle_init();
	  app_event_loop();
	  handle_deinit();
}
