
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


	

工作原理：
	在zubo程序中会进行一系列初始化工作

	mkfifo /tmp/mpfifo
	mkfifo /tmp/mpfifor
	mkfifo /tmp/snapfifo
	airsmart-mplayer -novideo -slave -idle -quiet -cache 1024 -srate 44100 -ao pcm:file=/tmp/snapfifo -input file=/tmp/mpfifo & 
	echo "loadfile http://music.163.com/song/media/outer/url?id=513791211.mp3" > /tmp/mpfifo"

	创建管道，然后让播放器airsmart-mplayer往管道里面写音频数据，可以通过loadfile切换新的歌曲

	roomcast程序会往管道里面读音频数据，然后按照协议编码成mp2或者flac，每个包打上时间戳后发送给每个连接的客户端，客户端启动会请求和服务器同步时间，计算出自	己和服务器时间的差异值，估算出服务器时间，然后用当前时间减去包的时间戳，如果和缓存时间差异不大，说明到了播放的时间，开始播放，服务器播放也是一样
	