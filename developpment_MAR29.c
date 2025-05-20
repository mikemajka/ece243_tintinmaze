#include <stdbool.h>

#define ps_2_base 0xFF200100
#define led_base 0xFF200000
#define MAP_WIDTH 177
#define MAP_HEIGHT 132

int player_x = 3;
int player_y = 114;
int player_angle = 1;  // user direction, 1 is facing front, -1 is facing
                       // back, 2 left, -2 right
int rotation_dir = 1;
int rotation_index = 0;
int image[260][195];

#include "game_finish_page.h"
#include "game_map.h"
#include "game_start_page.h"
#include "map.h"
#include "midgame.h"
#include "raycast.h"

int samples_n = 8000;

struct audio_t {
  volatile unsigned int control;
  volatile unsigned char rarc;
  volatile unsigned char ralc;
  volatile unsigned char warc;
  volatile unsigned char walc;
  volatile unsigned int ldata;
  volatile unsigned int rdata;
};

struct audio_t* const audiop = ((struct audio_t*)0xff203040);

#include "audio_data.h"

void audio_playback_mono(int* samples, int n);

int runner_pos[2] = {261, 38};
int runner_pos_prev[2] = {261, 38};
// this is array is used for helping to clear the player from previous theme
int mini_map_display_box[2] = {261, 28};
int runner_pixel_color = 0xF818;
int cloud_color = 0xACD2;
short int Buffer1[240][512];  // 240 rows, 512 (320 + padding) columns
short int Buffer2[240][512];
void plot_pixel(int x, int y, short int line_color);
void clear_screen();
void wait_for_vsync();
void plot_player(int position_x, int position_y, int pixel_color);
void clear_player_pixel(int position_x, int position_y);
bool legal_motion_detection_and_move(int position_x, int position_y,
                                     int keyboard_code);
void minimap_tracing(int player_x, int player_y);
void plot_minimap_area(int player_x, int player_y, int minimap_x,
                       int minimap_y);

bool game_ended(int player_pos_x, int player_pos_y);
void game_ended_hold();  // hold the program until user decided to restart the
                         // game

int pixel_buffer_start;
// initialize pixel_ctr_ptr to 0xFF203020
volatile int* pixel_ctr_ptr = (int*)0xFF203020;
int main(void) {
  // for moving key board
  char data_from_keyboard = 0;
  int ps2_data;
  int ravlid;
  volatile int* led_ptr = (int*)led_base;
  volatile int* ps_2_ptr = (int*)ps_2_base;
  *(ps_2_ptr) = 0xFF;
  *(led_ptr) = 0x00;
  // for moving key board

  *(pixel_ctr_ptr + 1) = (int)&Buffer1;
  wait_for_vsync();  // now buffer 1 in the front buffer slot
  pixel_buffer_start = *pixel_ctr_ptr;

  clear_screen();  // clear the buffer 1
  *(pixel_ctr_ptr + 1) = (int)&Buffer2;
  pixel_buffer_start = *(pixel_ctr_ptr + 1);
  clear_screen();  // clear the buffer of buffer 2
                   // both clear buffer

  for (int i = 0; i < 320; i++) {
    for (int j = 0; j < 240; j++) {
      short int background_color = start_screen[j][i] >> 16;
      plot_pixel(i, j, background_color);
    }
  }
  wait_for_vsync();
  pixel_buffer_start = *(pixel_ctr_ptr + 1);

  for (int i = 0; i < 320; i++) {
    for (int j = 0; j < 240; j++) {
      short int background_color = start_screen[j][i] >> 16;
      plot_pixel(i, j, background_color);
    }
  }

  wait_for_vsync();
  pixel_buffer_start = *(pixel_ctr_ptr + 1);

  while (1) {
    // keep getting input from keyboard to see if user want to start
    ps2_data = *(ps_2_ptr);
    ravlid = (ps2_data & 0x8000);
    if (ravlid) {
      data_from_keyboard = ps2_data & 0xFF;
      *(led_ptr) = data_from_keyboard;
      if (data_from_keyboard == 0b11110000) {
        int check_direction;  // find what is next after the break code
        check_direction = *(ps_2_ptr);
        while ((check_direction & 0x8000) == 0) {
          check_direction = *(ps_2_ptr);
        }
        check_direction = check_direction & 0xFF;
        if (check_direction == 0b00101001) {
          break;
        }
      }
    }
  }

  // if user press space, start the game
  samples_n = 8000;
  audio_playback_mono(audio_intro, samples_n);
  // more to wirte here
  // draw the background
  // actually draw the background for buffer 2
  for (int i = 0; i < 320; i++) {
    for (int j = 0; j < 240; j++) {
      if (i >= runner_pos[0] && i < runner_pos[0] + 4 && j >= runner_pos[1] &&
          j < runner_pos[1] + 4) {
     
        plot_pixel(i, j, runner_pixel_color);
      
      } else if (i >= mini_map_display_box[0] &&
                 i < mini_map_display_box[0] + 16 &&
                 j >= mini_map_display_box[1] &&
                 j < mini_map_display_box[1] + 16) {
        short int background_color = game_map[j][i] >> 16;
        plot_pixel(i, j, background_color);
      } else if (i >= 261 && j <= 43) {
        plot_pixel(i, j, cloud_color);
      } else {
        short int background_color = mid_game[j][i] >> 16;
        plot_pixel(i, j, background_color);
      }
    }
  }
  wait_for_vsync();

  pixel_buffer_start = *(pixel_ctr_ptr + 1);
  // draw the background on the next buffer
  // this is actually drawing the game pixel on buffer 1
  for (int i = 0; i < 320; i++) {
    for (int j = 0; j < 240; j++) {
      if (i >= runner_pos[0] && i < runner_pos[0] + 4 && j >= runner_pos[1] &&
          j < runner_pos[1] + 4) {
        plot_pixel(i, j, runner_pixel_color);
        
      } else if (i >= mini_map_display_box[0] &&
                 i < mini_map_display_box[0] + 16 &&
                 j >= mini_map_display_box[1] &&
                 j < mini_map_display_box[1] + 16) {
        short int background_color = game_map[j][i] >> 16;
        plot_pixel(i, j, background_color);
      } else if (i >= 261 && j <= 43) {
        plot_pixel(i, j, cloud_color);
      } else {
        short int background_color = mid_game[j][i] >> 16;
        plot_pixel(i, j, background_color);
      }
    }
  }
  wait_for_vsync();
  pixel_buffer_start = *(pixel_ctr_ptr + 1);
  // initalization is stop at this point
  // address of buffer 1 is now stored at address 0xFF203020
  // address of buffer 2 is at address 0xFF203024

  while (1) {
    ps2_data = *(ps_2_ptr);
    ravlid = (ps2_data & 0x8000);
    if (ravlid) {
      data_from_keyboard = ps2_data & 0xFF;
      *(led_ptr) = data_from_keyboard;
      if (data_from_keyboard == 0b11100000) {
        int check_direction;  // find what is next after the break code
        check_direction = *(ps_2_ptr);
        while ((check_direction & 0x8000) == 0) {
          check_direction = *(ps_2_ptr);
        }
        check_direction = check_direction & 0xFF;
        legal_motion_detection_and_move(
            runner_pos[0], runner_pos[1],
            check_direction);  // update player position
        player_x = (((runner_pos[0] + 2) - 260) * 3);
        player_y = ((runner_pos[1] + 2) * 3);
        minimap_tracing(runner_pos[0], runner_pos[1]);  // update display box
      }
    }
    // clear_screen();
    plot_minimap_area(runner_pos[0], runner_pos[1], mini_map_display_box[0],
                      mini_map_display_box[1]);
    render_scene();

    for (int i = 0; i < 260; i++) {
      for (int j = 0; j < 195; j++) {
        plot_pixel(i, j + 45, image[i][j]);
      }
    }
    wait_for_vsync();
    pixel_buffer_start = *(pixel_ctr_ptr + 1);

    if (game_ended(runner_pos[0], runner_pos[1])) {
      game_ended_hold();
    }
  }
}

void game_ended_hold() {
  volatile int* pixel_ctr_ptr = (int*)0xFF203020;
  char data_from_keyboard = 0;
  int ps2_data;
  int ravlid;
  volatile int* led_ptr = (int*)led_base;
  volatile int* ps_2_ptr = (int*)ps_2_base;
  *(ps_2_ptr) = 0xFF;
  *(led_ptr) = 0x00;

  for (int i = 0; i < 320; i++) {
    for (int j = 0; j < 240; j++) {
      short int background_color = game_finish_page[j][i] >> 16;
      plot_pixel(i, j, background_color);
    }
  }
  wait_for_vsync();
  pixel_buffer_start = *(pixel_ctr_ptr + 1);

  for (int i = 0; i < 320; i++) {
    for (int j = 0; j < 240; j++) {
      short int background_color = game_finish_page[j][i] >> 16;
      plot_pixel(i, j, background_color);
    }
  }
  wait_for_vsync();
  pixel_buffer_start = *(pixel_ctr_ptr + 1);
  samples_n = 19539;
  audio_playback_mono(audio_outro, samples_n);
  while (1) {
    // keep getting input from keyboard to see if user want to restart
    ps2_data = *(ps_2_ptr);
    ravlid = (ps2_data & 0x8000);
    if (ravlid) {
      data_from_keyboard = ps2_data & 0xFF;
      *(led_ptr) = data_from_keyboard;
      if (data_from_keyboard == 0b11110000) {
        int check_direction;  // find what is next after the break code
        check_direction = *(ps_2_ptr);
        while ((check_direction & 0x8000) == 0) {
          check_direction = *(ps_2_ptr);
        }
        check_direction = check_direction & 0xFF;
        if (check_direction == 0b00101001) {
          break;
        }
      }
    }
  }

  // set back player's initial posotion
  runner_pos[0] = 261;
  runner_pos[1] = 38;
  mini_map_display_box[0] = 261;
  mini_map_display_box[1] = 28;

  for (int i = 0; i < 320; i++) {
    for (int j = 0; j < 240; j++) {
      if (i >= runner_pos[0] && i < runner_pos[0] + 4 && j >= runner_pos[1] &&
          j < runner_pos[1] + 4) {
        plot_pixel(i, j, runner_pixel_color);
      } else if (i >= mini_map_display_box[0] &&
                 i < mini_map_display_box[0] + 16 &&
                 j >= mini_map_display_box[1] &&
                 j < mini_map_display_box[1] + 16) {
        short int background_color = game_map[j][i] >> 16;
        plot_pixel(i, j, background_color);
      } else if (i >= 261 && j <= 43) {
        plot_pixel(i, j, cloud_color);
      } else {
        short int background_color = mid_game[j][i] >> 16;
        plot_pixel(i, j, background_color);
      }
    }
  }
  wait_for_vsync();

  pixel_buffer_start = *(pixel_ctr_ptr + 1);
  // draw the background on the next buffer
  // this is actually drawing the game pixel on buffer 1
  for (int i = 0; i < 320; i++) {
    for (int j = 0; j < 240; j++) {
      if (i >= runner_pos[0] && i < runner_pos[0] + 4 && j >= runner_pos[1] &&
          j < runner_pos[1] + 4) {
        plot_pixel(i, j, runner_pixel_color);
      } else if (i >= mini_map_display_box[0] &&
                 i < mini_map_display_box[0] + 16 &&
                 j >= mini_map_display_box[1] &&
                 j < mini_map_display_box[1] + 16) {
        short int background_color = game_map[j][i] >> 16;
        plot_pixel(i, j, background_color);
      } else if (i >= 261 && j <= 43) {
        plot_pixel(i, j, cloud_color);
      } else {
        short int background_color = mid_game[j][i] >> 16;
        plot_pixel(i, j, background_color);
      }
    }
  }
  wait_for_vsync();
  pixel_buffer_start = *(pixel_ctr_ptr + 1);

  // point at the back buffer and return it
  return;
}

void plot_pixel(int x, int y, short int line_color) {
  volatile short int* one_pixel_address;
  one_pixel_address = (pixel_buffer_start + (y << 10) + (x << 1));
  *one_pixel_address = line_color;
  return;
}

void clear_screen() {
  int x;
  int y;
  for (x = 0; x < 320; x++) {
    for (y = 0; y < 240; y++) {
      plot_pixel(x, y, 0);
    }
  }
  return;
}

void wait_for_vsync() {
  volatile int* pixel_ctr_ptr = (int*)0xff203020;
  int status;
  *pixel_ctr_ptr = 1;
  status = *(pixel_ctr_ptr + 3);
  while ((status & 0x01) != 0) {
    status = *(pixel_ctr_ptr + 3);
  }
}

void plot_player(int position_x, int position_y, int pixel_color) {
  for (int i = 0; i <= 3; i++) {
    for (int j = 0; j <= 3; j++) {
      plot_pixel(position_x + i, position_y + j, pixel_color);
    }
  }
}

void clear_player_pixel(int position_x, int position_y) {
  for (int i = 0; i <= 3; i++) {
    for (int j = 0; j <= 3; j++) {
      plot_pixel(position_x + i, position_y + j, 0xFFFF);
    }
  }
}

void plot_minimap_area(int player_x, int player_y, int minimap_x,
                       int minimap_y) {
  for (int i = 261; i <= 319; i++) {
    for (int j = 0; j <= 43; j++) {
      if (i >= player_x && i < player_x + 4 && j >= player_y &&
          j < player_y + 4) {
        plot_pixel(i, j, runner_pixel_color);
      } else if (i >= minimap_x && i < minimap_x + 16 && j >= minimap_y &&
                 j < minimap_y + 16) {
        short int background_color = game_map[j][i] >> 16;
        plot_pixel(i, j, background_color);
      } else {
        plot_pixel(i, j, cloud_color);
      }
    }
  }
}

// this function check each time when a movement is detected, will check if the
// move is valid or not, return true if move is legal, vice versa
bool legal_motion_detection_and_move(int position_x, int position_y,
                                     int keyboard_code) {
  runner_pos_prev[0] = runner_pos[0];
  runner_pos_prev[1] = runner_pos[1];

  if (player_angle == -1) {  // currently looking down
    if (keyboard_code == 0b01101011 /*left key*/) {
      runner_pos[0] = runner_pos[0] + 1;

      // first check if this location is illegal
      for (int i = 0; i <= 3; i++) {
        for (int j = 0; j <= 3; j++) {
          int tempX = runner_pos[0] + i;
          int tempY = runner_pos[1] + j;
          if (game_map[tempY][tempX] == 0x5EEA0000) {
            runner_pos[0] = runner_pos[0] - 1;
            return false;
          } else if (tempX >= 320 || tempY < 0) {
            runner_pos[0] = runner_pos[0] - 1;
            return false;
          }
        }
      }
      // change player pov to looking right, -2
      rotation_dir = 0;
      for (rotation_index = 0; rotation_index <= 390; rotation_index += 10) {
        render_scene_rot();
        for (int i = 0; i < 260; i++) {
          for (int j = 0; j < 195; j++) {
            plot_pixel(i, j + 45, image[i][j]);
          }
        }
        wait_for_vsync();
        pixel_buffer_start = *(pixel_ctr_ptr + 1);
      }
      player_angle = -2;
      return true;

    } else if (keyboard_code == 0b01110100 /*right key*/) {
      runner_pos[0] = runner_pos[0] - 1;
      // player look right

      // first check if this location is illegal
      for (int i = 0; i <= 3; i++) {
        for (int j = 0; j <= 3; j++) {
          int tempX = runner_pos[0] + i;
          int tempY = runner_pos[1] + j;
          if (game_map[tempY][tempX] == 0x5EEA0000) {
            runner_pos[0] = runner_pos[0] + 1;
            return false;
          } else if (tempX >= 320 || tempY < 0) {
            runner_pos[0] = runner_pos[0] + 1;
            return false;
          }
        }
      }

      rotation_dir = 1;
      for (rotation_index = 0; rotation_index <= 390;
           rotation_index += 10) {  // left turn 90
        render_scene_rot();
        for (int i = 0; i < 260; i++) {
          for (int j = 0; j < 195; j++) {
            plot_pixel(i, j + 45, image[i][j]);
          }
        }
        wait_for_vsync();
        pixel_buffer_start = *(pixel_ctr_ptr + 1);
      }
      player_angle = 2;
      return true;

    } else if (keyboard_code == 0b01110101 /*up key*/) {
      runner_pos[1] = runner_pos[1] + 1;
      // first check if this location is illegal
      for (int i = 0; i <= 3; i++) {
        for (int j = 0; j <= 3; j++) {
          int tempX = runner_pos[0] + i;
          int tempY = runner_pos[1] + j;
          if (game_map[tempY][tempX] == 0x5EEA0000) {
            runner_pos[1] = runner_pos[1] - 1;
            return false;
          } else if (tempX >= 320 || tempY < 0) {
            runner_pos[1] = runner_pos[1] - 1;
            return false;
          }
        }
      }

      player_angle = -1;
      return true;

    } else if (keyboard_code == 0b01110010 /*down key*/) {
      runner_pos[1] = runner_pos[1] - 1;

      // first check if this location is illegal
      for (int i = 0; i <= 3; i++) {
        for (int j = 0; j <= 3; j++) {
          int tempX = runner_pos[0] + i;
          int tempY = runner_pos[1] + j;
          if (game_map[tempY][tempX] == 0x5EEA0000) {
            runner_pos[1] = runner_pos[1] + 1;
            return false;
          } else if (tempX >= 320 || tempY < 0) {
            runner_pos[1] = runner_pos[1] + 1;
            return false;
          }
        }
      }
      rotation_dir = 1;

      for (rotation_index = 0; rotation_index <= 390;
           rotation_index += 10) {  // left turn 90
        render_scene_rot();
        for (int i = 0; i < 260; i++) {
          for (int j = 0; j < 195; j++) {
            plot_pixel(i, j + 45, image[i][j]);
          }
        }
        wait_for_vsync();
        pixel_buffer_start = *(pixel_ctr_ptr + 1);
      }
      player_angle = 2;

      rotation_dir = 1;

      for (rotation_index = 0; rotation_index <= 390;
           rotation_index += 10) {  // left turn 90
        render_scene_rot();
        for (int i = 0; i < 260; i++) {
          for (int j = 0; j < 195; j++) {
            plot_pixel(i, j + 45, image[i][j]);
          }
        }
        wait_for_vsync();
        pixel_buffer_start = *(pixel_ctr_ptr + 1);
      }

      player_angle = 1;
      return true;
    }
  } else if (player_angle == 1) {  // currently looking up
    if (keyboard_code == 0b01101011 /*left key*/) {
      runner_pos[0] = runner_pos[0] - 1;

      // first check if this location is illegal
      for (int i = 0; i <= 3; i++) {
        for (int j = 0; j <= 3; j++) {
          int tempX = runner_pos[0] + i;
          int tempY = runner_pos[1] + j;
          if (game_map[tempY][tempX] == 0x5EEA0000) {
            runner_pos[0] = runner_pos[0] + 1;
            return false;
          } else if (tempX >= 320 || tempY < 0) {
            runner_pos[0] = runner_pos[0] + 1;
            return false;
          }
        }
      }
      // change player pov to looking right, -2
      rotation_dir = 0;
      for (rotation_index = 0; rotation_index <= 390; rotation_index += 10) {
        render_scene_rot();
        for (int i = 0; i < 260; i++) {
          for (int j = 0; j < 195; j++) {
            plot_pixel(i, j + 45, image[i][j]);
          }
        }
        wait_for_vsync();
        pixel_buffer_start = *(pixel_ctr_ptr + 1);
      }
      player_angle = 2;
      return true;

    } else if (keyboard_code == 0b01110100 /*right key*/) {
      runner_pos[0] = runner_pos[0] + 1;
      // player look right

      // first check if this location is illegal
      for (int i = 0; i <= 3; i++) {
        for (int j = 0; j <= 3; j++) {
          int tempX = runner_pos[0] + i;
          int tempY = runner_pos[1] + j;
          if (game_map[tempY][tempX] == 0x5EEA0000) {
            runner_pos[0] = runner_pos[0] - 1;
            return false;
          } else if (tempX >= 320 || tempY < 0) {
            runner_pos[0] = runner_pos[0] - 1;
            return false;
          }
        }
      }

      rotation_dir = 1;
      for (rotation_index = 0; rotation_index <= 390;
           rotation_index += 10) {  // left turn 90
        render_scene_rot();
        for (int i = 0; i < 260; i++) {
          for (int j = 0; j < 195; j++) {
            plot_pixel(i, j + 45, image[i][j]);
          }
        }
        wait_for_vsync();
        pixel_buffer_start = *(pixel_ctr_ptr + 1);
      }
      player_angle = -2;
      return true;

    } else if (keyboard_code == 0b01110101 /*up key*/) {
      runner_pos[1] = runner_pos[1] - 1;
      // first check if this location is illegal
      for (int i = 0; i <= 3; i++) {
        for (int j = 0; j <= 3; j++) {
          int tempX = runner_pos[0] + i;
          int tempY = runner_pos[1] + j;
          if (game_map[tempY][tempX] == 0x5EEA0000) {
            runner_pos[1] = runner_pos[1] + 1;
            return false;
          } else if (tempX >= 320 || tempY < 0) {
            runner_pos[1] = runner_pos[1] + 1;
            return false;
          }
        }
      }

      player_angle = 1;
      return true;

    } else if (keyboard_code == 0b01110010 /*down key*/) {
      runner_pos[1] = runner_pos[1] + 1;

      // first check if this location is illegal
      for (int i = 0; i <= 3; i++) {
        for (int j = 0; j <= 3; j++) {
          int tempX = runner_pos[0] + i;
          int tempY = runner_pos[1] + j;
          if (game_map[tempY][tempX] == 0x5EEA0000) {
            runner_pos[1] = runner_pos[1] - 1;
            return false;
          } else if (tempX >= 320 || tempY < 0) {
            runner_pos[1] = runner_pos[1] - 1;
            return false;
          }
        }
      }
      rotation_dir = 1;

      for (rotation_index = 0; rotation_index <= 390;
           rotation_index += 10) {  // left turn 90
        render_scene_rot();
        for (int i = 0; i < 260; i++) {
          for (int j = 0; j < 195; j++) {
            plot_pixel(i, j + 45, image[i][j]);
          }
        }
        wait_for_vsync();
        pixel_buffer_start = *(pixel_ctr_ptr + 1);
      }
      player_angle = 2;

      rotation_dir = 1;

      for (rotation_index = 0; rotation_index <= 390;
           rotation_index += 10) {  // left turn 90
        render_scene_rot();
        for (int i = 0; i < 260; i++) {
          for (int j = 0; j < 195; j++) {
            plot_pixel(i, j + 45, image[i][j]);
          }
        }
        wait_for_vsync();
        pixel_buffer_start = *(pixel_ctr_ptr + 1);
      }

      player_angle = -1;
      return true;
    }
  } else if (player_angle == 2) {  // currently looking left
    if (keyboard_code == 0b01101011 /*left key*/) {
      runner_pos[1] = runner_pos[1] + 1;

      // first check if this location is illegal
      for (int i = 0; i <= 3; i++) {
        for (int j = 0; j <= 3; j++) {
          int tempX = runner_pos[0] + i;
          int tempY = runner_pos[1] + j;
          if (game_map[tempY][tempX] == 0x5EEA0000) {
            runner_pos[1] = runner_pos[1] - 1;
            return false;
          } else if (tempX >= 320 || tempY < 0) {
            runner_pos[1] = runner_pos[1] - 1;
            return false;
          }
        }
      }
      // change player pov to looking right, -2
      rotation_dir = 0;
      for (rotation_index = 0; rotation_index <= 390; rotation_index += 10) {
        render_scene_rot();
        for (int i = 0; i < 260; i++) {
          for (int j = 0; j < 195; j++) {
            plot_pixel(i, j + 45, image[i][j]);
          }
        }
        wait_for_vsync();
        pixel_buffer_start = *(pixel_ctr_ptr + 1);
      }
      player_angle = -1;
      return true;

    } else if (keyboard_code == 0b01110100 /*right key*/) {
      runner_pos[1] = runner_pos[1] - 1;
      // player look right

      // first check if this location is illegal
      for (int i = 0; i <= 3; i++) {
        for (int j = 0; j <= 3; j++) {
          int tempX = runner_pos[0] + i;
          int tempY = runner_pos[1] + j;
          if (game_map[tempY][tempX] == 0x5EEA0000) {
            runner_pos[1] = runner_pos[1] + 1;
            return false;
          } else if (tempX >= 320 || tempY < 0) {
            runner_pos[1] = runner_pos[1] + 1;
            return false;
          }
        }
      }

      rotation_dir = 1;
      for (rotation_index = 0; rotation_index <= 390;
           rotation_index += 10) {  // left turn 90
        render_scene_rot();
        for (int i = 0; i < 260; i++) {
          for (int j = 0; j < 195; j++) {
            plot_pixel(i, j + 45, image[i][j]);
          }
        }
        wait_for_vsync();
        pixel_buffer_start = *(pixel_ctr_ptr + 1);
      }
      player_angle = 1;
      return true;

    } else if (keyboard_code == 0b01110101 /*up key*/) {
      runner_pos[0] = runner_pos[0] - 1;
      // first check if this location is illegal
      for (int i = 0; i <= 3; i++) {
        for (int j = 0; j <= 3; j++) {
          int tempX = runner_pos[0] + i;
          int tempY = runner_pos[1] + j;
          if (game_map[tempY][tempX] == 0x5EEA0000) {
            runner_pos[0] = runner_pos[0] + 1;
            return false;
          } else if (tempX >= 320 || tempY < 0) {
            runner_pos[0] = runner_pos[0] + 1;
            return false;
          }
        }
      }

      player_angle = 2;
      return true;

    } else if (keyboard_code == 0b01110010 /*down key*/) {
      runner_pos[0] = runner_pos[0] + 1;

      // first check if this location is illegal
      for (int i = 0; i <= 3; i++) {
        for (int j = 0; j <= 3; j++) {
          int tempX = runner_pos[0] + i;
          int tempY = runner_pos[1] + j;
          if (game_map[tempY][tempX] == 0x5EEA0000) {
            runner_pos[0] = runner_pos[0] - 1;
            return false;
          } else if (tempX >= 320 || tempY < 0) {
            runner_pos[0] = runner_pos[0] - 1;
            return false;
          }
        }
      }
      rotation_dir = 1;

      for (rotation_index = 0; rotation_index <= 390;
           rotation_index += 10) {  // left turn 90
        render_scene_rot();
        for (int i = 0; i < 260; i++) {
          for (int j = 0; j < 195; j++) {
            plot_pixel(i, j + 45, image[i][j]);
          }
        }
        wait_for_vsync();
        pixel_buffer_start = *(pixel_ctr_ptr + 1);
      }
      player_angle = 1;

      rotation_dir = 1;

      for (rotation_index = 0; rotation_index <= 390;
           rotation_index += 10) {  // left turn 90
        render_scene_rot();
        for (int i = 0; i < 260; i++) {
          for (int j = 0; j < 195; j++) {
            plot_pixel(i, j + 45, image[i][j]);
          }
        }
        wait_for_vsync();
        pixel_buffer_start = *(pixel_ctr_ptr + 1);
      }

      player_angle = -2;
      return true;
    }
  } else if (player_angle == -2) {  // currently looking right
    if (keyboard_code == 0b01101011 /*left key*/) {
      runner_pos[1] = runner_pos[1] - 1;

      // first check if this location is illegal
      for (int i = 0; i <= 3; i++) {
        for (int j = 0; j <= 3; j++) {
          int tempX = runner_pos[0] + i;
          int tempY = runner_pos[1] + j;
          if (game_map[tempY][tempX] == 0x5EEA0000) {
            runner_pos[1] = runner_pos[1] + 1;
            return false;
          } else if (tempX >= 320 || tempY < 0) {
            runner_pos[1] = runner_pos[1] + 1;
            return false;
          }
        }
      }
      // change player pov to looking right, -2
      rotation_dir = 0;
      for (rotation_index = 0; rotation_index <= 390; rotation_index += 10) {
        render_scene_rot();
        for (int i = 0; i < 260; i++) {
          for (int j = 0; j < 195; j++) {
            plot_pixel(i, j + 45, image[i][j]);
          }
        }
        wait_for_vsync();
        pixel_buffer_start = *(pixel_ctr_ptr + 1);
      }
      player_angle = 1;
      return true;

    } else if (keyboard_code == 0b01110100 /*right key*/) {
      runner_pos[1] = runner_pos[1] + 1;
      // player look right

      // first check if this location is illegal
      for (int i = 0; i <= 3; i++) {
        for (int j = 0; j <= 3; j++) {
          int tempX = runner_pos[0] + i;
          int tempY = runner_pos[1] + j;
          if (game_map[tempY][tempX] == 0x5EEA0000) {
            runner_pos[1] = runner_pos[1] - 1;
            return false;
          } else if (tempX >= 320 || tempY < 0) {
            runner_pos[1] = runner_pos[1] - 1;
            return false;
          }
        }
      }

      rotation_dir = 1;
      for (rotation_index = 0; rotation_index <= 390;
           rotation_index += 10) {  // left turn 90
        render_scene_rot();
        for (int i = 0; i < 260; i++) {
          for (int j = 0; j < 195; j++) {
            plot_pixel(i, j + 45, image[i][j]);
          }
        }
        wait_for_vsync();
        pixel_buffer_start = *(pixel_ctr_ptr + 1);
      }
      player_angle = -1;
      return true;

    } else if (keyboard_code == 0b01110101 /*up key*/) {
      runner_pos[0] = runner_pos[0] + 1;
      // first check if this location is illegal
      for (int i = 0; i <= 3; i++) {
        for (int j = 0; j <= 3; j++) {
          int tempX = runner_pos[0] + i;
          int tempY = runner_pos[1] + j;
          if (game_map[tempY][tempX] == 0x5EEA0000) {
            runner_pos[0] = runner_pos[0] - 1;
            return false;
          } else if (tempX >= 320 || tempY < 0) {
            runner_pos[0] = runner_pos[0] - 1;
            return false;
          }
        }
      }

      player_angle = -2;
      return true;

    } else if (keyboard_code == 0b01110010 /*down key*/) {
      runner_pos[0] = runner_pos[0] - 1;

      // first check if this location is illegal
      for (int i = 0; i <= 3; i++) {
        for (int j = 0; j <= 3; j++) {
          int tempX = runner_pos[0] + i;
          int tempY = runner_pos[1] + j;
          if (game_map[tempY][tempX] == 0x5EEA0000) {
            runner_pos[0] = runner_pos[0] + 1;
            return false;
          } else if (tempX >= 320 || tempY < 0) {
            runner_pos[0] = runner_pos[0] + 1;
            return false;
          }
        }
      }
      rotation_dir = 1;

      for (rotation_index = 0; rotation_index <= 390;
           rotation_index += 10) {  // left turn 90
        render_scene_rot();
        for (int i = 0; i < 260; i++) {
          for (int j = 0; j < 195; j++) {
            plot_pixel(i, j + 45, image[i][j]);
          }
        }
        wait_for_vsync();
        pixel_buffer_start = *(pixel_ctr_ptr + 1);
      }
      player_angle = -1;

      rotation_dir = 1;

      for (rotation_index = 0; rotation_index <= 390;
           rotation_index += 10) {  // left turn 90
        render_scene_rot();
        for (int i = 0; i < 260; i++) {
          for (int j = 0; j < 195; j++) {
            plot_pixel(i, j + 45, image[i][j]);
          }
        }
        wait_for_vsync();
        pixel_buffer_start = *(pixel_ctr_ptr + 1);
      }

      player_angle = 2;
      return true;
    }
  }
}

void minimap_tracing(int player_x, int player_y) {
  mini_map_display_box[0] = player_x - 5;
  mini_map_display_box[1] = player_y - 5;

  if (mini_map_display_box[0] < 261) {
    mini_map_display_box[0] = 261;

  } else if (mini_map_display_box[0] > 304) {
    mini_map_display_box[0] = 304;
  }

  if (mini_map_display_box[1] < 0) {
    mini_map_display_box[1] = 0;
  } else if (mini_map_display_box[1] > 28) {
    mini_map_display_box[1] = 28;
  }
  return;
}

bool game_ended(int player_pos_x, int player_pos_y) {
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      int pixel_x = player_pos_x + i;
      int pixel_y = player_pos_y + j;
      if ((pixel_x == 317 && (pixel_y == 1 || pixel_y == 2)) ||
          (pixel_x == 318 && ((pixel_y == 1) || (pixel_y == 2)))) {
        return true;
      }
    }
  }
  return false;
}

void audio_playback_mono(int* samples, int n) {
  int data;

  audiop->control = 0x8;  // clear the output FIFOs
  audiop->control = 0x0;  // resume input conversion
  for (int i = 0; i < n; i++) {
    // wait till there is space in the output FIFO
    while (audiop->warc == 0);

    data = samples[i];

    audiop->rdata = data;
    audiop->ldata = data;
  }
}
