#include "pebble.h"

#define STEP_MS 1000
#define ARRAY_SIZE 100000

static Window *window;
static GRect window_frame;

static Layer *canvas;
static AppTimer *timer;

static int seconds = 0;
static int minutes = 0;
static int hours = 0;

static int seconds_x = 0, seconds_y = 120;
static int minutes_x = 0, minutes_y = 65;
static int hours_x = 0, hours_y = 30;

// An array for storing grayscale intensity values.
//static int * pixel;

/**
 * http://stackoverflow.com/questions/1100090/looking-for-an-efficient-integer-square-root-algorithm-for-arm-thumb2
 * \brief    Fast Square root algorithm
 *
 * Fractional parts of the answer are discarded. That is:
 *      - SquareRoot(3) --> 1
 *      - SquareRoot(4) --> 2
 *      - SquareRoot(5) --> 2
 *      - SquareRoot(8) --> 2
 *      - SquareRoot(9) --> 3
 *
 * \param[in] a_nInput - unsigned integer for which to find the square root
 *
 * \return Integer square root of the input value.
 */
uint32_t SquareRoot(uint32_t a_nInput)
{
    uint32_t op  = a_nInput;
    uint32_t res = 0;
    uint32_t one = 1uL << 30; // The second-to-top bit is set: use 1u << 14 for uint16_t type; use 1uL<<30 for uint32_t type


    // "one" starts at the highest power of four <= than the argument.
    while (one > op)
    {
        one >>= 2;
    }

    while (one != 0)
    {
        if (op >= res + one)
        {
            op = op - (res + one);
            res = res +  2 * one;
        }
        res >>= 1;
        one >>= 2;
    }
    return res;
}

static int func(int x)
{
  return SquareRoot(x)/2;
}

/*
static void setPixel(int x, int y, int val, int * pixel)
{
  int index = y*window_frame.size.w  + x;
  
  pixel[index] = val;
}

static int getPixel(int x, int y, int * pixel)
{
  int index = y*window_frame.size.w  + x;
  
  if(index < ARRAY_SIZE)
  {
    return pixel[index];
  }
  return 20;
}*/

// Converts form 1-100 grayscale to 0/1 monochrome.
/*
static int grayscale_to_monochrome(int val)
{
  return val <= 50 ? 0 : 1;
}
*/

static void draw_func(Layer *me, GContext *ctx)
{
  
  //int pixela[ARRAY_SIZE];// = (int*)malloc(sizeof(int)*ARRAY_SIZE);
  //int* pixel = pixela;
  
  int w = window_frame.size.w;
  int h = window_frame.size.h;
    
  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_draw_rect(ctx, window_frame);
  
  graphics_context_set_stroke_color(ctx, GColorWhite);
  
  // Populate the image in grayscale space.
  for(int y = 0; y < h; y++)
  for(int x = 0; x < w; x++)
  {
    int dist_final;
    
    // Seconds.
    int dx = x - seconds_x;
    int dy = y - seconds_y;
    int dist = dx*dx + dy*dy;
    dist = func(dist);
    dist_final = dist;
    
    // Minutes.
    dx = x - minutes_x;
    dy = y - minutes_y;
    dist = dx*dx + dy*dy;
    dist = func(dist);
    
    if(dist_final > dist)
    {
      dist_final = dist;
    }
    
    
    // Hours.
    dx = x - hours_x;
    dy = y - hours_y;
    dist = dx*dx + dy*dy;
    dist = func(dist);
    
    if(dist_final > dist)
    {
      dist_final = dist;
    }
    
    
    if(dist_final <= 1)
    {
      dist_final = 1;
    }
    
    if(dist_final >= 100)
   {
      dist_final = 100;   
    }
    
    
    if(dist_final % 2 == 0)
    {
      GPoint p = GPoint(x, y);
      graphics_draw_pixel(ctx, p);
    }
    
    
    // Set the grascale color.
    //setPixel(x, y, dist_final, pixel);
    
    
  }
  
  /*
  // Convert to a representation on the screen using Floyd-steinberg dithering.
  for(int y = 0; y < h; y++)
  for(int x = 0; x < w; x++)
  {
    
      int oldpixel  = getPixel(x, y, pixel);//pixel[x][y];
      int newpixel  = grayscale_to_monochrome(oldpixel);
      
      int quant_error  = oldpixel - (newpixel*100);
    
      bool xb = x < w - 1;
      bool yb = y < h - 1;
    
      // Now with new and improved bounds checking.
    
      if(xb)
      setPixel(x+1, y, getPixel(x+1, y, pixel) + quant_error * 7/16, pixel); 
      if(yb)
      {
        if(x > 0)
        setPixel(x-1, y + 1, getPixel(x-1, y+1, pixel) + quant_error * 3/16, pixel); 
        
        setPixel(x, y + 1, getPixel(x, y+1, pixel) + quant_error * 5/16, pixel); 
        if(xb)
        setPixel(x+1, y + 1, getPixel(x+1, y+1, pixel) + quant_error * 1/16, pixel); 
      }
         
      // Draw using the given context. 
      if(newpixel == 1)
      {
        GPoint p = GPoint(x, y);
        graphics_draw_pixel(ctx, p);
      }
      
      
  }// End of Dithering loop.
  */
    
}

static void update_func(void *data)
{

  // -- Update
  int w = window_frame.size.w;
  seconds_x = w * seconds/60;
  minutes_x = w * minutes/60;
  hours_x = w * hours/24;
  
  
  layer_mark_dirty(canvas);
  timer = app_timer_register(STEP_MS, update_func, NULL);
}

// Keeps track of the current time.
static void tick_handler(struct tm *tick_time, TimeUnits units_changed)
{
  // Get a tm structure
  time_t temp = time(NULL); 
  tick_time = localtime(&temp);

  // Populate each of the correct data fields.
  seconds = tick_time -> tm_sec;
  minutes = tick_time -> tm_min;
  hours   = tick_time -> tm_hour;
}


static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect frame = window_frame = layer_get_frame(window_layer);

  //pixel = (int*)malloc(sizeof(int)*frame.size.w*frame.size.h);
  
  canvas = layer_create(frame);
  layer_set_update_proc(canvas, draw_func);
  layer_add_child(window_layer, canvas);


  // -- Data initialization.
}

static void window_unload(Window *window)
{
  layer_destroy(canvas);
}



static void init(void) {
      
  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload
  });
  window_stack_push(window, true /* Animated */);
  window_set_background_color(window, GColorBlack);

  //accel_data_service_subscribe(0, NULL);

  timer = app_timer_register(STEP_MS, update_func, NULL);
  tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
  
  
}

static void deinit(void) {
  //accel_data_service_unsubscribe();

  window_destroy(window);

}

int main(void) {
  init();
  app_event_loop();
  deinit();
}


