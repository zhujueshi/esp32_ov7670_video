/*
  * ESPRESSIF MIT License
  *
  * Copyright (c) 2017 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
  *
  * Permission is hereby granted for use on ESPRESSIF SYSTEMS products only, in which case,
  * it is free of charge, to any person obtaining a copy of this software and associated
  * documentation files (the "Software"), to deal in the Software without restriction, including
  * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
  * and/or sell copies of the Software, and to permit persons to whom the Software is furnished
  * to do so, subject to the following conditions:
  *
  * The above copyright notice and this permission notice shall be included in all copies or
  * substantial portions of the Software.
  *
  * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
  * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
  * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
  * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
  * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
  *
  */
#ifndef _IOT_CAMERA_TASK_H_
#define _IOT_CAMERA_TASK_H_


#define WIFI_PASSWORD CONFIG_WIFI_PASSWORD
#define WIFI_SSID     CONFIG_WIFI_SSID
/**
 * CAMERA_PF_RGB565 = 0,       //!< RGB, 2 bytes per pixel
 * CAMERA_PF_YUV422 = 1,       //!< YUYV, 2 bytes per pixel
 * CAMERA_PF_GRAYSCALE = 2,    //!< 1 byte per pixel
 * CAMERA_PF_JPEG = 3,         //!< JPEG compressed
 * CAMERA_PF_RGB555 = 4,       //!< RGB, 2 bytes per pixel
 * CAMERA_PF_RGB444 = 5,       //!< RGB, 2 bytes per pixel
 */
#define CAMERA_PIXEL_FORMAT CAMERA_PF_RGB565

/*
 * CAMERA_FS_QQVGA = 4,     //!< 160x120
 * CAMERA_FS_HQVGA = 7,     //!< 240x160
 * CAMERA_FS_QCIF = 6,      //!< 176x144
 * CAMERA_FS_QVGA = 8,      //!< 320x240
 * CAMERA_FS_VGA = 10,      //!< 640x480
 * CAMERA_FS_SVGA = 11,     //!< 800x600
 */
#define CAMERA_FRAME_SIZE CAMERA_FS_QVGA

/**
 * @breif call xSemaphoreTake to take camera_sem
 */
void take_camera_sem();

/**
 * @breif call xSemaphoreGive to give camera_sem
 */
void give_camera_sem();

void app_lcd_init(void);
void app_lcd_task(void *pvParameters);
void http_server_task(void *pvParameters);

#endif
