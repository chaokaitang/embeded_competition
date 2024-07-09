/*
 * Copyright (c) 2022 HiSilicon (Shanghai) Technologies CO., LIMITED.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef IOT_CONFIG_H
#define IOT_CONFIG_H

// <CONFIG THE LOG
/* if you need the iot log for the development , please enable it, else please comment it */
#define CONFIG_LINKLOG_ENABLE 1

// < CONFIG THE WIFI
/* Please modify the ssid and pwd for the own */
#define CONFIG_AP_SSID "hispark"    // WIFI SSID
#define CONFIG_AP_PWD "12345678" // WIFI PWD
/* Tencent iot Cloud user ID , password */
#define CONFIG_USER_ID "664f6c3cc2c0cb155cbe0cd3_Hispark"
#define CONFIG_USER_PWD "d57e0f7a454700ad7fa0b9e81d7c20f8e688bfdfe3663033233fb501f504772c"
#define CN_CLIENTID "664f6c3cc2c0cb155cbe0cd3_Hispark_0_0_2024052318" // Tencent cloud ClientID format: Product ID + device name
#endif