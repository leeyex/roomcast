
roomcast单独使用 

	作为服务器 ./roomcast 0 [mp2/flac]   //不选默认为mp2
	作为客户端 ./roomcast 1 server_ip    // server_ip 为服务器地址

zubo使用
	（roomcast 要放在系统路径里面）
	在开机脚本中启动就行 ./zubo [mp2/flac] // 不选默认启动roomcast为mp2  
	zubo工作流程，启动后会组播寻找roomcast服务器，寻找服务器的过程会查找，
	    三秒内找到服务器，则启动roomcast客户端，多个roomcast服务器只会连第一个找到的，
							如果roomcast服务器ip变更(如断网重连），会自动重新连接新的ip地址
            三秒后找不到服务器，则自己启动roomcast服务器

	如果两个zubo同时起来，可能会发生都启动roomcast服务器的情况

	
	zubo程序会检测当前是否旋钮在aux位置，在的话，才会继续运行直至启动roomcast，否则等待，
	如果由aux旋钮转到其他位置，则reboot
