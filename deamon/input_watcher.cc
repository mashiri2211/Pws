#include "lib/input_watcher.hpp"
#include "lib/utils.hpp"

void InputWatcher::AddInterval( ChronoClock::time_point &last_interval )
{
     ChronoClock::time_point current_time = ChronoClock::now();

     int diff_interval = std::chrono::duration_cast
          < chrono_millisec > (current_time - last_interval).count();

     if( diff_interval > chrono_millisec(1400).count() )
          diff_interval = 1400; // this first press keys

     this->current_stat->all_interval += diff_interval;
     this->current_stat->num_pressed_keys++;

     last_interval = current_time;
}


float InputWatcher::input_stat::CanculateCPM()
{
     return CanculateCPS() * 60;
}

float InputWatcher::input_stat::CanculateCPS()
{
     float mean_interv = (float)(this->all_interval) / 
          (float)(this->num_pressed_keys);
     
     return 1000 / mean_interv;
}


void InputWatcher::StartEventTimer()
{
     std::thread t( [ this ](){
               
               while( 1 )
               {
                    std::this_thread::sleep_for( std::chrono::hours(1) );

                    this->stat_per_hours.push_back( new struct input_stat );
                    this->current_stat = (*(--this->stat_per_hours.end()));
               }
          });
}


void InputWatcher::HandlerKeyPress()
{

#ifdef __linux__
     struct input_event ev;
     int fd;
    
     fd = open("/dev/input/event0", O_RDONLY);
     //fd = open("/dev/input/event8", O_RDONLY); // for another keyboard

     ChronoClock::time_point last_interval = ChronoClock::now();

     this->StartEventTimer(); 

     while( 1 )
     {
          
          read(fd, &ev, sizeof(ev));
          if( (ev.type == EV_KEY) && (ev.value == 0) )
               this->AddInterval( last_interval );
          
          std::this_thread::sleep_for( std::chrono::milliseconds(10) );

     }
     
     close( fd );
     
#endif
}

InputWatcher::InputWatcher()
{
     this->stat_per_hours.push_back( new struct input_stat );
     this->current_stat = (*(--this->stat_per_hours.end()));

     this->input_thread = std::thread(&InputWatcher::HandlerKeyPress,
               this);
}
