#include "mathlib.h"

#define SCREEN_WIDTH 260
#define SCREEN_HEIGHT 195



#define FOV_ANGLE 60
#define NUM_ANGLES 260  // same is screen width

int get_wall_color(int distance);
void cast_ray(float ray_dir_x, float ray_dir_y, int *height, int *distance);
void render_scene();
void render_scene_rot();

int get_wall_color(int distance) {
  int brightness = 255 - 2 * (distance);  // adjust brightness coefficient
  if (brightness < 0) brightness = 0;

  int red = (brightness >> 3) & 0x1F;
  int green = (brightness >> 2) & 0x3F;
  int blue = (brightness >> 3) & 0x1F;

  return (red << 11) | (green << 5) | blue;
}

// Cast a single ray to detect a wall
void cast_ray(float ray_dir_x, float ray_dir_y, int *height, int *distance) {
  int ray_x = player_x;
  int ray_y = player_y;

  float ray_xf = player_x;
  float ray_yf = player_y;

  while (ray_x >= 0 && ray_x < MAP_WIDTH && ray_y >= 0 && ray_y < MAP_HEIGHT) {
    if (map[ray_y][ray_x] ==0x3FC6) {
      // Calculate distance
      float sum_of_squares = abs(ray_x - player_x) * abs(ray_x - player_x) +
                             abs(ray_y - player_y) * abs(ray_y - player_y);
      float r = sum_of_squares / 2.0;  // Initial guess
      for (int k = 0; k < 10; k++) {
        r = 0.5 * (r + sum_of_squares / r);  // Babylonian method
      }

      *distance = (int)r;

      *height = 10 * (SCREEN_HEIGHT) /
                *distance;  // scale height of wall, ADJUST COEEFFICIENT

      return;
    }

    ray_xf += ray_dir_x;
    ray_yf += ray_dir_y;
    ray_x = (int)ray_xf;
    ray_y = (int)ray_yf;
  }

  // No wall detected for debug
  *distance = 0;
  *height = 0;
}

void render_scene() {  // Iterate over the screen columns (rays)

  float ray_dir_x = 0, ray_dir_y = 0;

  for (int i = 0; i < SCREEN_WIDTH; ++i) {
    if (player_angle == 1) {  // select direction and feed angles to cast ray
      if (i < 129) {
        ray_dir_x = -sin_table[130 - i - 1];
        ray_dir_y = -cos_table[130 - i - 1];

      } else {
        ray_dir_x = sin_table[i - 129];
        ray_dir_y = -cos_table[i - 129];
      }
    } else if (player_angle == -1) {
      if (i < 129) {
        ray_dir_x = sin_table[130 - i - 1];
        ray_dir_y = cos_table[130 - i - 1];

      } else {
        ray_dir_x = -sin_table[i - 129];
        ray_dir_y = cos_table[i - 129];
      }
    } else if (player_angle == 2) {
      if (i < 129) {
        ray_dir_y = sin_table[130 - i - 1];
        ray_dir_x = -cos_table[130 - i - 1];

      } else {
        ray_dir_y = -sin_table[i - 129];
        ray_dir_x = -cos_table[i - 129];
      }
    } else {  // -2
      if (i < 129) {
        ray_dir_y = -sin_table[130 - i - 1];
        ray_dir_x = cos_table[130 - i - 1];

      } else {
        ray_dir_y = sin_table[i - 129];
        ray_dir_x = cos_table[i - 129];
      }
    }
    int height = 0, distance = 0;

    cast_ray(ray_dir_x, ray_dir_y, &height, &distance);

    int color = get_wall_color(distance);

    int wall_top = 97 - (int)(height / 2);

    if (wall_top < 0) {
      wall_top = 0;
      height = 195;
    }
    for (int l = 0; l < 195; l++) {
      image[i][wall_top + l] = 0x0;
    }

    for (int j = 0; j < height; j++) {
      image[i][wall_top + j] = color;
    }
  }
}

void render_scene_rot() {  // Iterate over the screen columns (rays)

  float ray_dir_x = 0, ray_dir_y = 0;

  for (int i = 0; i < SCREEN_WIDTH; ++i) {
    if (player_angle == 1) {    // select direction and feed angles to cast ray
      if (rotation_dir == 0) {  // facing up and turning left

        if (rotation_index < 130) {
          if (i < 129 + rotation_index) {
            ray_dir_x = -sin_table[130 - i - 1 + rotation_index];
            ray_dir_y = -cos_table[130 - i - 1 + rotation_index];

          } else {
            ray_dir_x = sin_table[i - 129 - rotation_index];
            ray_dir_y = -cos_table[i - 129 - rotation_index];
          }
        } else if (rotation_index < 260) {
          ray_dir_x = -sin_table[130 - i - 1 + rotation_index];
          ray_dir_y = -cos_table[130 - i - 1 + rotation_index];

        } else if (rotation_index >= 260) {
          if (i < 130 - (390 - rotation_index)) {
            ray_dir_x = -sin_table[390 - (-i + rotation_index + 130 - 390)];
            ray_dir_y = cos_table[390 - (-i + rotation_index + 130 - 390)];

          } else {
            ray_dir_x = -sin_table[130 - i - 1 + rotation_index];
            ray_dir_y = -cos_table[130 - i - 1 + rotation_index];
          }
        }

      } else {  // facing up and turnin right
        if (rotation_index < 130) {
          if (i < 129 - rotation_index) {
            ray_dir_x = -sin_table[130 - i - 1 - rotation_index];
            ray_dir_y = -cos_table[130 - i - 1 - rotation_index];

          } else {
            ray_dir_x = sin_table[i - 129 + rotation_index];
            ray_dir_y = -cos_table[i - 129 + rotation_index];
          }
        } else if (rotation_index < 260) {
          ray_dir_x = sin_table[i - 129 + rotation_index];
          ray_dir_y = -cos_table[i - 129 + rotation_index];

        } else if (rotation_index >= 260) {
          if (i < 130 + (390 - rotation_index)) {
            ray_dir_x = sin_table[i - 129 + rotation_index];
            ray_dir_y = -cos_table[i - 129 + rotation_index];

          } else {
            ray_dir_x = sin_table[390 - (rotation_index - 130 + i - 390)];
            ray_dir_y = cos_table[390 - (rotation_index - 130 + i - 390)];
          }
        }
      }

    } else if (player_angle == -1) {
      if (rotation_dir == 0) {  // facing down and turning left

        if (rotation_index < 130) {
          if (i < 129 + rotation_index) {
            ray_dir_x = sin_table[130 - i - 1 + rotation_index];
            ray_dir_y = cos_table[130 - i - 1 + rotation_index];

          } else {
            ray_dir_x = -sin_table[i - 129 - rotation_index];
            ray_dir_y = cos_table[i - 129 + rotation_index];
          }
        } else if (rotation_index < 260) {
          ray_dir_x = sin_table[130 - i - 1 + rotation_index];
          ray_dir_y = cos_table[130 - i - 1 + rotation_index];

        } else if (rotation_index >= 260) {
          if (i < 130 - (390 - rotation_index)) {
            ray_dir_x = sin_table[390 - (-i + rotation_index + 130 - 390)];
            ray_dir_y = -cos_table[390 - (-i + rotation_index + 130 - 390)];

          } else {
            ray_dir_x = sin_table[130 - i - 1 + rotation_index];
            ray_dir_y = cos_table[130 - i - 1 + rotation_index];
          }
        }

      } else {  // facing down and turnin right
        if (rotation_index < 130) {
          if (i < 129 - rotation_index) {
            ray_dir_x = sin_table[130 - i - 1 - rotation_index];
            ray_dir_y = cos_table[130 - i - 1 - rotation_index];

          } else {
            ray_dir_x = -sin_table[i - 129 + rotation_index];
            ray_dir_y = cos_table[i - 129 + rotation_index];
          }
        } else if (rotation_index < 260) {
          ray_dir_x = -sin_table[i - 129 + rotation_index];
          ray_dir_y = cos_table[i - 129 + rotation_index];

        } else if (rotation_index >= 260) {
          if (i < 130 + (390 - rotation_index)) {
            ray_dir_x = -sin_table[i - 129 + rotation_index];
            ray_dir_y = cos_table[i - 129 + rotation_index];

          } else {
            ray_dir_x = -sin_table[390 - (rotation_index - 130 + i - 390)];
            ray_dir_y = -cos_table[390 - (rotation_index - 130 + i - 390)];
          }
        }
      }
    } else if (player_angle == 2) {
      if (rotation_dir == 0) {  // facing left and turning left

        if (rotation_index < 130) {
          if (i < 129 + rotation_index) {
            ray_dir_y = sin_table[130 - i - 1 + rotation_index];
            ray_dir_x = cos_table[130 - i - 1 + rotation_index];

          } else {
            ray_dir_y = -sin_table[i - 129 - rotation_index];
            ray_dir_x = cos_table[i - 129 + rotation_index];
          }
        } else if (rotation_index < 260) {
          ray_dir_y = sin_table[130 - i - 1 + rotation_index];
          ray_dir_x = cos_table[130 - i - 1 + rotation_index];

        } else if (rotation_index >= 260) {
          if (i < 130 - (390 - rotation_index)) {
            ray_dir_y = sin_table[390 - (-i + rotation_index + 130 - 390)];
            ray_dir_x = -cos_table[390 - (-i + rotation_index + 130 - 390)];

          } else {
            ray_dir_y = sin_table[130 - i - 1 + rotation_index];
            ray_dir_x = cos_table[130 - i - 1 + rotation_index];
          }
        }

      } else {  // facing left and turnin right
        if (rotation_index < 130) {
          if (i < 129 - rotation_index) {
            ray_dir_y = sin_table[130 - i - 1 - rotation_index];
            ray_dir_x = cos_table[130 - i - 1 - rotation_index];

          } else {
            ray_dir_y = -sin_table[i - 129 + rotation_index];
            ray_dir_x = cos_table[i - 129 + rotation_index];
          }
        } else if (rotation_index < 260) {
          ray_dir_y = -sin_table[i - 129 + rotation_index];
          ray_dir_x = -cos_table[i - 129 + rotation_index];

        } else if (rotation_index >= 260) {
          if (i < 130 + (390 - rotation_index)) {
            ray_dir_y = -sin_table[i - 129 + rotation_index];
            ray_dir_x = -cos_table[i - 129 + rotation_index];

          } else {
            ray_dir_y = -sin_table[rotation_index + 130 - i];
            ray_dir_x = cos_table[rotation_index + 130 - i];
          }
        }
      }
    } else if (player_angle == -2) {  // -2
      if (rotation_dir == 0) {        // facing right and turning left

        if (rotation_index < 130) {
          if (i < 129 + rotation_index) {
            ray_dir_y = -sin_table[130 - i - 1 + rotation_index];
            ray_dir_x = cos_table[130 - i - 1 + rotation_index];

          } else {
            ray_dir_y = sin_table[i - 129 - rotation_index];
            ray_dir_x = cos_table[i - 129 - rotation_index];
          }
        } else if (rotation_index < 260) {
          ray_dir_y = -sin_table[130 - i - 1 + rotation_index];
          ray_dir_x = cos_table[130 - i - 1 + rotation_index];

        } else if (rotation_index >= 260) {
          if (i < 130 - (390 - rotation_index)) {
            ray_dir_y = -cos_table[rotation_index - 260 - i];
            ray_dir_x = -sin_table[rotation_index - 260 - i];

          } else {
            ray_dir_y = -sin_table[130 - i - 1 + rotation_index];
            ray_dir_x = cos_table[130 - i - 1 + rotation_index];
          }
        }

      } else {  // facing right and turnin right
        if (rotation_index < 130) {
          if (i < 129 - rotation_index) {
            ray_dir_y = -sin_table[130 - i - 1 - rotation_index];
            ray_dir_x = cos_table[130 - i - 1 - rotation_index];

          } else {
            ray_dir_y = sin_table[i - 129 + rotation_index];
            ray_dir_x = cos_table[i - 129 + rotation_index];
          }
        } else if (rotation_index < 260) {
          ray_dir_y = sin_table[i - 129 + rotation_index];
          ray_dir_x = cos_table[i - 129 + rotation_index];

        } else if (rotation_index >= 260) {
          if (i < 130 + (390 - rotation_index)) {
            ray_dir_y = sin_table[i - 129 + rotation_index];
            ray_dir_x = cos_table[i - 129 + rotation_index];  // bug

          } else {
            ray_dir_y = sin_table[390 - (rotation_index - 130 + i - 390)];
            ray_dir_x = -cos_table[390 - (rotation_index - 130 + i - 390)];
          }
        }
      }
    }

    int height = 0, distance = 0;

    cast_ray(ray_dir_x, ray_dir_y, &height, &distance);

    int color = get_wall_color(distance);

    int wall_top = 97 - (int)(height / 2);

    if (wall_top < 0) {
      wall_top = 0;
      height = 195;
    }

    for (int l = 0; l < 195; l++) {
      image[i][wall_top + l] = 0x0;
    }

    for (int j = 0; j < height; j++) {
      image[i][wall_top + j] = color;
    }
  }
}