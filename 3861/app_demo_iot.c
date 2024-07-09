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
// Rearranged by XFT & XKF
#include <hi_task.h>
#include <string.h>
#include <hi_wifi_api.h>
#include <hi_mux.h>
#include <hi_io.h>
#include <hi_gpio.h>
#include "iot_config.h"
#include "iot_log.h"
#include "iot_main.h"
#include "iot_profile.h"
#include "ohos_init.h"
#include "cmsis_os2.h"
#include <stdio.h>
#include <unistd.h>
#include "hi_timer.h"
#include "hi_time.h"
#include "iot_watchdog.h"
#include "iot_errno.h"
#include "hi_errno.h"
#include "iot_gpio.h"
#include "ssd1306_fonts.h"
#include "ssd1306.h"
#include <unistd.h>
#include <stdint.h>
#include "hi_uart.h"
#include "uart.h"

#include "app_demo_uart.h"

/* attribute initiative to report */
#define TAKE_THE_INITIATIVE_TO_REPORT
#define ONE_SECOND (1000)
/* oc request id */
#define CN_COMMADN_INDEX "commands/request_id="
#define CONTROL_NODE_1 "ControlNodeONE"
#define CONTROL_NODE_STATE_1 "LED_ON"
#define CONTROL_NODE_STATE_2 "LED_OFF"
char g_ligthStatus[16]="init";
char x_strdata[16]="init";
char y_strdata[16]="init";
char z_strdata[16]="init";
char tmp_strdata[16]="Init";




char str[16]={0};

typedef void (*FnMsgCallBack)(hi_gpio_value val);

typedef struct FunctionCallback
{
    hi_bool stop;
    hi_u32 conLost;
    hi_u32 queueID;
    hi_u32 iotTaskID;
    FnMsgCallBack msgCallBack;
} FunctionCallback;
FunctionCallback g_functinoCallback;



 
char* Int2String(int num,char *str)//10进制 
{
    int i = 0;//指示填充str 
    if(num<0)//如果num为负数，将num变正 
    {
        num = -num;
        str[i++] = '-';
    } 
    //转换 
    do
    {
        str[i++] = num%10+48;//取num最低位 字符0~9的ASCII码是48~57；简单来说数字0+48=48，ASCII码对应字符'0' 
        num /= 10;//去掉最低位    
    }while(num);//num不为0继续循环
    
    str[i] = '\0';
    
    //确定开始调整的位置 
    int j = 0;
    if(str[0]=='-')//如果有负号，负号不用调整 
    {
        j = 1;//从第二位开始调整 
        ++i;//由于有负号，所以交换的对称轴也要后移1位 
    }
    //对称交换 
    for(;j<i/2;j++)
    {
        //对称交换两端的值 其实就是省下中间变量交换a+b的值：a=a+b;b=a-b;a=a-b; 
        str[j] = str[j] + str[i-1-j];
        str[i-1-j] = str[j] - str[i-1-j];
        str[j] = str[j] - str[i-1-j];
    } 
    
    return str;//返回转换后的值 
}





static void sg_3_0(){
    unsigned int time = 20000;

    // hi_io_set_func(HI_IO_NAME_GPIO_4, HI_IO_FUNC_GPIO_4_GPIO);
    // hi_gpio_set_dir(HI_GPIO_IDX_4, HI_GPIO_DIR_OUT);
    // hi_gpio_set_ouput_val(HI_GPIO_IDX_4, IOT_GPIO_VALUE1);

    IoTGpioInit(5);
    IoTGpioSetDir(5, IOT_GPIO_DIR_OUT);
    hi_io_set_func(HI_IO_NAME_GPIO_5, HI_IO_FUNC_GPIO_5_GPIO);
    IoTGpioSetOutputVal(5, IOT_GPIO_VALUE1);
    hi_udelay(500);
    IoTGpioSetOutputVal(5, IOT_GPIO_VALUE0);
    hi_udelay(time - 500);
}


/* CPU Sleep time Set */
unsigned int TaskMsleepa(unsigned int ms)
{
    if (ms <= 0)
    {
        return HI_ERR_FAILURE;
    }
    return hi_sleep((hi_u32)ms);
}

static void DeviceConfigInit(hi_gpio_value val)
{
    hi_io_set_func(HI_IO_NAME_GPIO_9, HI_IO_FUNC_GPIO_9_GPIO);
    hi_gpio_set_dir(HI_GPIO_IDX_9, HI_GPIO_DIR_OUT);
    hi_gpio_set_ouput_val(HI_GPIO_IDX_9, val);
}

static int DeviceMsgCallback(FnMsgCallBack msgCallBack)
{
    g_functinoCallback.msgCallBack = msgCallBack;
    return 0;
}

static void wechatControlDeviceMsg(hi_gpio_value val)
{
    DeviceConfigInit(val);
}

// < this is the callback function, set to the mqtt, and if any messages come, it will be called
// < The payload here is the json string
static void DemoMsgRcvCallBack(int qos, const char *topic, const char *payload)
{
    IOT_LOG_DEBUG("RCVMSG:QOS:%d TOPIC:%s PAYLOAD:%s\r\n", qos, topic, payload);
    /* 云端下发命令后，板端的操作处理 */
    printf("%s\n", payload);


    IoTProfileService_t service;
    IoTProfileKV_t property;
    printf("%s\n", strstr(payload, CONTROL_NODE_1));

    if (strstr(payload, CONTROL_NODE_1) != NULL) // 识别该payload命令名称
    {
        if (strstr(payload, CONTROL_NODE_STATE_2) != NULL) // 识别该payload命令内容
        {

            wechatControlDeviceMsg(HI_GPIO_VALUE1);
            strcpy(g_ligthStatus,"LED_FF");
            // 识别该指令之后你的对应操作
        }
        else if (strstr(payload, CONTROL_NODE_STATE_1) != NULL) // 识别该payload命令内容
        {

            // 识别该指令之后你的对应操作
            wechatControlDeviceMsg(HI_GPIO_VALUE0);
            strcpy(g_ligthStatus,"LED_OFN");
        }
    }
    const char *requesID;
    char *tmp;
    IoTCmdResp_t resp;
    tmp = strstr(topic, CN_COMMADN_INDEX);
    if (tmp != NULL)
    {
        // /< now you could deal your own works here --THE COMMAND FROM THE PLATFORM
        // /< now er roport the command execute result to the platform
        requesID = tmp + strlen(CN_COMMADN_INDEX);
        resp.requestID = requesID;
        resp.respName = NULL;
        resp.retCode = 0; ////< which means 0 success and others failed
        resp.paras = NULL;
        (void)IoTProfileCmdResp(CONFIG_USER_ID, &resp);
    }
    return HI_NULL;
}
hi_void IotPublishSample(void) // 上报函数举例1
{
    

    // sg_3_0();

    // ssd1306_printf("recbuf success!\n");

    // float tmp = ((float)((buf[8] << 4) | (buf[9]>>4)) *0.0625);

    // int16_t x_16=(buf[3] << 8) | buf[2];
    // int16_t y_16=(buf[5] << 8) | buf[4];
    // int16_t z_16=(buf[7] << 8) | buf[6];

    // float x_ac = 0.004*x_16;
    // float y_ac = 0.004*y_16;
    // float z_ac = 0.004*z_16;

    // sprintf(x_strdata,"%.2f",x_ac);
    // sprintf(y_strdata,"%.2f",y_ac);
    // sprintf(z_strdata,"%.2f",z_ac);
    // sprintf(tmp_strdata,"%.2f",tmp);


    IoTProfileService_t service;
    IoTProfileKV_t property;
    IoTProfileKV_t property1;
    IoTProfileKV_t property2;
    IoTProfileKV_t property3;
    unsigned char status = 0;
    memset_s(&property, sizeof(property), 0, sizeof(property));
    memset_s(&property1, sizeof(property1), 0, sizeof(property1));
    memset_s(&property2, sizeof(property2), 0, sizeof(property2));
    memset_s(&property3, sizeof(property3), 0, sizeof(property3));
    property.type = EN_IOT_DATATYPE_STRING; // 上报属性类型
    property.key = "head_pose";          // 上报属性名称
    property.value = tmp_strdata;                 // 上报属性的值
    
    property.nxt=&property1;

    property1.type=EN_IOT_DATATYPE_STRING;
    property1.key = "raw";
    property1.value = x_strdata;

    property1.nxt=&property2;
    property2.type=EN_IOT_DATATYPE_STRING;
    property2.key = "pitch";
    property2.value = y_strdata;

    property2.nxt=&property3;
    property3.type= EN_IOT_DATATYPE_STRING;
    property3.key = "roll";
    property3.value = z_strdata;



    memset_s(&service, sizeof(service), 0, sizeof(service));
    service.serviceID = "sensor";
    service.serviceProperty = &property;
    IoTProfilePropertyReport(CONFIG_USER_ID, &service);
}
hi_void IotPublishSampletest(void) // 上报函数举例2
{
    IoTProfileService_t service;
    IoTProfileKV_t property;
    unsigned char status = 0;

    memset_s(&property, sizeof(property), 0, sizeof(property));
    property.type = EN_IOT_DATATYPE_INT;
    property.key = "TEST";
    property.iValue = 1;
    memset_s(&service, sizeof(service), 0, sizeof(service));
    service.serviceID = "TEST";
    service.serviceProperty = &property;
    IoTProfilePropertyReport(CONFIG_USER_ID, &service);
}
static hi_void *DemoEntry(const char *arg)
{
    WifiStaReadyWait();
    cJsonInit();
    oled_init();
    printf("start");

    // IoTGpioInit(4);
    // IoSetFunc(4, 5); /* 设置IO5的功能 */
    // IoTGpioSetDir(4, IOT_GPIO_DIR_OUT);
    // IoTPwmInit(1);
    // IoTPwmStart(1, 50, 40000);
    // sg_3_0();
    IoTMain();
    ssd1306_printf("iot init success!\n");
    /* 云端下发回调 */
    IoTSetMsgCallback(DemoMsgRcvCallBack);

    /* 主动上报 */
#ifdef TAKE_THE_INITIATIVE_TO_REPORT
    while (1)
    {
        /* 用户可以在这调用发布函数进行发布，需要用户自己写调用函数 */
        IotPublishSample(); // 发布例程
                            // ssd1306_printf("upload success!\n");

#endif
        TaskMsleepa(ONE_SECOND);
    }
    return NULL;
}

// < This is the demo entry, we create a task here,
// and all the works has been done in the demo_entry
#define CN_IOT_TASK_STACKSIZE 0x1000
#define CN_IOT_TASK_PRIOR 24
#define CN_IOT_TASK_NAME "IOTDEMO"

static void AppDemoIot(void)
{
    osThreadAttr_t attr;
    IoTWatchDogDisable();

    attr.name = "IOTDEMO";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 10240;
    attr.priority = CN_IOT_TASK_PRIOR;

    if (osThreadNew((osThreadFunc_t)DemoEntry, NULL, &attr) == NULL)
    {
        printf("[mqtt] Falied to create IOTDEMO!\n");
    }
}

SYS_RUN(AppDemoIot);