
#define GPIO_NAME_SIZE 20
#define GPIO_DATE_SIZE 20

#define GPIO_OUTPUT_HIGN	 0x01
#define GPIO_OUTPUT_LOW 	 0x00
#define GPIO_LED_OFF	 0x01
#define GPIO_LED_ON 	 0x00

#define GPIO_PA(n) 	(0*32 + n)
#define GPIO_PB(n) 	(1*32 + n)
#define GPIO_PC(n) 	(2*32 + n)
#define GPIO_PD(n) 	(3*32 + n)
#define GPIO_PE(n) 	(4*32 + n)
#define GPIO_PF(n) 	(5*32 + n)

#define AIRSMART_SECCUSS 0
#define AIRSMART_FAILED  -1

typedef enum airsmart_gpio_function {
	AIRSA_GPIO_INPUT 	    = 0x00, 
	AIRSA_GPIO_OUTPUT 	= 0x01,   
	AIRSA_GPIO_LED_RED 	= 0x02,  
	AIRSA_GPIO_LED_GREED 	= 0x03,  
	AIRSA_GPIO_LED_BLUE 	= 0x04,  
	AIRSA_GPIO_LED_WHITE 	= 0x05,  
	AIRSA_GPIO_OUTPUT_MUTE 	= 0x06,  
	AIRSA_GPIO_INPUT_POWER 	= 0x07, 
	AIRSA_GPIO_INPUT_I2S 	= 0x08, 
	AIRSA_GPIO_SET_I2S 	= 0x09, 
	AIRSA_GPIO_GET_REGINFO  = 0x0a,
	AIRSA_GPIO_INPUT_AUX 	= 0x0b, 
	AIRSA_GPIO_WKUP_ENABLE 	= 0x0c, 
	AIRSA_GPIO_WKUP_DISABLE 	= 0x0d, 
	AIRSA_GPIO_RADIOOO_WKUP_ENABLE 	= 0x0e, 
	AIRSA_GPIO_RADIOOO_WKUP_DISABLE 	= 0x0f, 
	AIRSA_GPIO_MAX	= 0xff, 
}airsmart_gpio_function_e;

typedef enum airsmart_gpio_action {
	AIRSA_ACTION_POWEROFF_TIME 	    = 0x00, 
	AIRSA_ACTION_APPCOMPLETE_STATUS 	= 0x01,   
	AIRSA_ACTION_OTHER 	= 0x02,   
	AIRSA_ACTION_MAX	= 0xff, 
}airsmart_gpio_action_e;

typedef struct airsmart_gpio_extend {
	char date[GPIO_DATE_SIZE];
	airsmart_gpio_action_e gpio_action;
}airsmart_gpio_extend_t;

typedef enum airsmart_gpio_mode {
	AIRSA_GPIO_MODE_INPUT 	= 0x00, 
	AIRSA_GPIO_MODE_OUTPUT 	= 0x01,   
	AIRSA_GPIO_MODE_MAX	= 0xff, 
}airsmart_gpio_mode_e;

typedef struct airsmart_gpio_func {
	char name[GPIO_NAME_SIZE];
	airsmart_gpio_function_e gpio_function;
	airsmart_gpio_mode_e gpio_mode;
	unsigned int  gpio;
	struct airsmart_gpio_func *next;
}airsmart_gpio_func_t;


#ifdef __cplusplus 
extern "C" {
#endif

extern int airsmart_gpio_resume(void);
extern int airsmart_gpio_suspend(void);
extern int airsmart_fastpoweron_enable(void);
extern int airsmart_fastpoweron_disable(void);
extern int airsmart_battery_get_status(unsigned long gpio);


extern  int airsmart_gpio_dev_open(int *fd_airsmart,airsmart_gpio_function_e gpio_function);//打开gpio设备
extern  int airsmart_gpio_dev_set(int fd_airsmart,char value);//给gpio设置值
extern  int airsmart_gpio_dev_close(int fd_airsmart);//关闭gpio
extern void airsmart_gpio_init();//初始化使用的gpio
extern void airsmart_gpio_red_on();//红色指示灯亮
extern void airsmart_gpio_red_off();//红色指示灯熄灭
extern void airsmart_gpio_greed_on();//绿色指示灯亮
extern void airsmart_gpio_greed_off();//绿色指示灯熄灭
extern void airsmart_gpio_white_on();//白色指示灯亮
extern void airsmart_gpio_white_off();//白色指示灯熄灭
extern void airsmart_gpio_close();//关闭初始化的gpio设备
extern void airsmart_gpio_mute();//猫卡内核静音
extern void airsmart_gpio_no_mute();//猫卡内核设置非静音
extern void airsmart_gpio_i2s_mute();//关闭i2s数据
extern void airsmart_gpio_i2s_no_mute();//打开i2s数据
extern int airsmart_get_gpio_poweroff();
extern int airsmart_get_gpio_neihe_status();
extern void battery_flay_init();
extern void hano_set_airsmart_capacity(int t_airsmart_capacity);
extern int hano_get_airsmart_capacity();
void airsmart_gpio_i2s_disablemic();
void airsmart_gpio_i2s_enablemic();

#ifdef __cplusplus 
}
#endif

