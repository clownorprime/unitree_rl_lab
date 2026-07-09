#pragma once

#include <string>
#include <vector>
#include <deque>
#include <termios.h>
#include <unistd.h>
#include <thread>

#include <unitree/dds_wrapper/common/unitree_joystick.hpp>


/**
 * @brief Maintain a keyboard reading thread.
 * And get the latest key value.
 */
class Keyboard : public unitree::common::UnitreeJoystick
{
public:
  Keyboard()
  {
    LT.smooth = 1.0f;
    RT.smooth = 1.0f;

    tcgetattr( fileno( stdin ), &_oldSettings );
    _newSettings = _oldSettings;
    _oldSettings.c_lflag |= ( ICANON |  ECHO);
    _newSettings.c_lflag &= (~ICANON & ~ECHO);

    _startKey();

    _thread_running  = true;
    _readThread = std::thread([this] {
      while (_running) {
        _read();
      }
    });
  }

  ~Keyboard()
  {
    _thread_running = false;
    _pauseKey();
  }

  void update()
  {
    update_joystick_keys_(_key);

    if(_key != _last_key)
    {
      on_pressed = _key != "";
      on_released = _key == "";
    }
    else
    {
      on_pressed = false;
      on_released = false;
    }
    
    _last_key = _key;
  }

  /**
   * @brief Get the current key value
   * 
   * @return std::string 
   */
  std::string key() const { return _key; };

  /**
   * @brief Get the String object from keyboard 
   * 
   * @param slogan Used to prompt the user for input
   * @return std::string 
   */
  std::string getString(std::string slogan)
  {
    // Stop reading keyboard value
    _running = false;
    _pauseKey();

    std::string stringtemp;
    std::cout << slogan << std::endl;// prompt
    std::getline(std::cin, stringtemp);

    // Restart reading keyboard value
    _startKey();
    _running = true;

    return stringtemp;
  }

  /**
   * flags; available after update()
   */
  bool on_pressed = false;
  bool on_released = false;

  private:
  bool _thread_running = false;
  bool _running = false;
  std::thread _readThread;

  void _read()
  {
    if(_running)
    {
      FD_ZERO(&_fd_set);
      FD_SET( fileno(stdin), &_fd_set);

      _tv.tv_sec = 0;
      _tv.tv_usec = 80000;

      if(select(fileno(stdin)+1, &_fd_set, NULL, NULL, &_tv))
      {
        // Read the key value into _c
        int res = read( fileno(stdin), &_c, 1 );

        // Parser the key value
        if(_c != '\033') {
          // This is a normal key
          _key = _c;
        }else{
          // This is a special key
          int m = read(fileno(stdin), &_c, 1);
          if(_c == '[')
          {
            m = read(fileno(stdin), &_c, 1);
            switch (_c)
            {
            case 'A': _key = "up";    break;
            case 'B': _key = "down";  break;
            case 'C': _key = "right"; break;
            case 'D': _key = "left";  break;
            default:  _key = "";      break;
            }
          }
        }
      }else{
        _key = "";
      }
      // std::cout << "key: "<< key() << std::endl;
    }
  }

  /**
   * @brief Restore keyboard default settings.
   */
  void _pauseKey()
  {
    tcsetattr( fileno( stdin ), TCSANOW, &_oldSettings );
    _running = false;
  }

  /**
   * @brief Disable canonical mode and echoing of input characters.
   */
  void _startKey()
  {
    tcsetattr( fileno( stdin ), TCSANOW, &_newSettings );
    _running = true;
  }

  fd_set _fd_set;
  char _c = '\0';
  std::string _key, _last_key;
  
  termios _oldSettings, _newSettings;
  timeval _tv;

  void update_joystick_keys_(const std::string& key)
  {
    back(key == " " ? 1 : 0);
    start((key == "\n" || key == "\r") ? 1 : 0);
    LS(0);
    RS(0);
    LB(key == "q" ? 1 : 0);
    RB(key == "e" ? 1 : 0);
    A(key == "a" ? 1 : 0);
    B(key == "b" ? 1 : 0);
    X(key == "x" ? 1 : 0);
    Y(key == "y" ? 1 : 0);
    up(key == "up" ? 1 : 0);
    down(key == "down" ? 1 : 0);
    left(key == "left" ? 1 : 0);
    right(key == "right" ? 1 : 0);
    F1(key == "1" ? 1 : 0);
    F2(key == "2" ? 1 : 0);
    LT(key == "z" ? 1.0f : 0.0f);
    RT(key == "c" ? 1.0f : 0.0f);
    lx(0.0f);
    ly(0.0f);
    rx(0.0f);
    ry(0.0f);
  }
};