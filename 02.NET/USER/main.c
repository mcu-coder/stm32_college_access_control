#include "git.h"

// 软件定时器设定
static Timer task1_id;
static Timer task2_id;
static Timer task3_id;

// 获取全局变量
const char *topics[] = {S_TOPIC_NAME};

extern U8 bTimeout25ms; /** < 25毫秒定时器超时标志 */

// 硬件初始化
void Hardware_Init(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); // 设置中断优先级分组为组2：2位抢占优先级，2位响应优先级
    HZ = GB16_NUM();                                // 字数
    delay_init();                                   // 延时函数初始化
    GENERAL_TIM_Init(TIM_4, 0, 1);
   
		Usart2_Init(115200, 2, 2);// 人脸
 
	
#if OLED // OLED文件存在
    OLED_Init();
    OLED_ColorTurn(0);   // 0正常显示，1 反色显示
    OLED_DisplayTurn(0); // 0正常显示 1 屏幕翻转显示
#endif
		
    while (Reset_Threshole_Value(&threshold_value_init, &device_state_init) != MY_SUCCESSFUL)
        delay_ms(5); // 初始化阈值

		
		RC522_Init ();  //RC522初始
		PcdReset ();
		M500PcdConfigISOType ( 'A' );		/*设置工作方式*/ 
		
		TIM3_PWM_Init(20);   // PWM
		
	  Usart1_Init(9600); 				// 串口1初始化为9600 语音
  	JR6001_Init();     // 语音播报
#if OLED // OLED文件存在
    OLED_Clear();
#endif
}
// 网络初始化
void Net_Init()
{

#if OLED // OLED文件存在
    char str[50];
    OLED_Clear();
    // 写OLED内容
    sprintf(str, "-请打开WIFI热点");
    OLED_ShowCH(0, 0, (unsigned char *)str);
    sprintf(str, "-名称:%s         ", SSID);
    OLED_ShowCH(0, 2, (unsigned char *)str);
    sprintf(str, "-密码:%s         ", PASS);
    OLED_ShowCH(0, 4, (unsigned char *)str);
    sprintf(str, "-频率: 2.4 GHz   ");
    OLED_ShowCH(0, 6, (unsigned char *)str);
#endif
    ESP8266_Init();          // 初始化ESP8266
    while (DevLink()) // 接入OneNET
        delay_ms(300);
    while (Subscribe(topics, 1)) // 订阅主题
        delay_ms(300);

    Connect_Net = 60; // 入网成功
		

		
#if OLED              // OLED文件存在
    OLED_Clear();
#endif
}

// 任务1
void task1(void)
{
		
   	Automation_Close();

		if(device_state_init.speak)
		{
			// 语音反馈
			if(	device_state_init.speak == 2){
				// 注册
				JR6001_SongControl(device_state_init.speak ,1);
				Beep_time(100);
			}
			else if( device_state_init.speak == 3){
				// RFID绑定
				//JR6001_SongControl(device_state_init.speak ,1);
				Beep_time(100);
			}
			else if(device_state_init.speak == 4){
				// 密码
				JR6001_SongControl(device_state_init.speak ,1);
				Beep_time(100);
				
			}
			else if(device_state_init.speak == 5){
				// 人脸
				JR6001_SongControl(device_state_init.speak ,1);
				Beep_time(100);
				 
			}
			else  if(device_state_init.speak == 6){
				// RFID
				JR6001_SongControl(device_state_init.speak ,1);
				 Beep_time(100);
			}
			else  if(device_state_init.speak == 7){
				// 错误
				JR6001_SongControl(device_state_init.speak ,1);
				Beep_time(100);
			}

			device_state_init.speak = 0;
				
		}
	   
}
// 任务2
void task2(void)
{
    char str[50];
// 设备重连
#if NETWORK_CHAEK
    if (Connect_Net == 0) {
			
#if OLED // OLED文件存在
        OLED_Clear();
        // 写OLED内容
        sprintf(str, "-请打开WIFI热点");
        OLED_ShowCH(0, 0, (unsigned char *)str);
        sprintf(str, "-名称:%s         ", SSID);
        OLED_ShowCH(0, 2, (unsigned char *)str);
        sprintf(str, "-密码:%s         ", PASS);
        OLED_ShowCH(0, 4, (unsigned char *)str);
        sprintf(str, "-频率: 2.4 GHz   ");
        OLED_ShowCH(0, 6, (unsigned char *)str);
#endif
        ESP8266_Init();          // 初始化ESP8266
      while (DevLink()) // 接入云平台
        delay_ms(300);
			while (Subscribe(topics, 1)) // 订阅主题
        delay_ms(300);
        Connect_Net = 60; // 入网成功
#if OLED                  // OLED文件存在
        OLED_Clear();
#endif
    }
#endif
 
}
// 任务3
void task3(void)
{
    // 10s发一次
    if (Connect_Net && Data_init.App == 0) {
        Data_init.App = 1;
    }

		//JR6001_SongControl(7,1);
}
// 软件初始化
void SoftWare_Init(void)
{
    // 定时器初始化
    timer_init(&task1_id, task1, 1000, 1); // 每1s上传一次设备数据
    timer_init(&task2_id, task2, 300, 1);    // 跟新数据包
    timer_init(&task3_id, task3, 30000, 1);  // 每10秒发送一次数据到客户端

    timer_start(&task1_id);
    timer_start(&task2_id);
    timer_start(&task3_id);
	
	
}
// 主函数
int main(void)
{

    unsigned char *dataPtr = NULL;
    SoftWare_Init(); // 软件初始化
    Hardware_Init(); // 硬件初始化
    // 启动提示
    Beep_time(100);
    Net_Init();            // 网络初始
    TIM_Cmd(TIM4, ENABLE); // 使能计数器
	
		OLED_Clear();
    OLED_ShowCH(16, 1, "KA键密码验证");
    OLED_ShowCH(16, 3, "KB键人脸验证");
		OLED_ShowCH(16, 5, "KC键RFID验证");
	
    while (1) {

         
    }
}
