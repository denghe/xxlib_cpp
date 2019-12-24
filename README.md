系统：
ubuntu 18.04 LTS server，安装时勾安 OpenSSH server 默认安全选项

安装编译器，依赖库：
sudo apt install gcc-8 g++-8 gdb gdbserver libboost-context-dev libuv1-dev uuid-dev libsqlite3-dev libmariadb-dev libreadline-dev

设置 gcc g++ 命令指向 8.0:
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-8 60 --slave /usr/bin/g++ g++ /usr/bin/g++-8

解除 linux fd 限制: /etc/security/limits.conf 追加下面的内容, 保存重启
root hard nofile 65535
root soft nofile 65535
root soft core unlimited
root hard core unlimited
* hard nofile 65535
* soft nofile 65535
* soft core unlimited
* hard core unlimited




如果网络异常, 可通过代理安装:
sudo apt-get -o Acquire::http::proxy="http://xxxxxxxxxxxxxx:xxx/" install ..............

如果希望连接 WSL, 继续安装 ssh zip
修改 /etc/ssh/sshd_config, 开启 Port Addresxxxxxx 那几个, 以及 PasswordAuthentication 改为 yes
并 sudo service ssh restart
如果报类似下列错误:
Could not load host key: /etc/ssh/ssh_host_ed25519_key
...
修正：
sudo ssh-keygen -t rsa -f /etc/ssh/ssh_host_rsa_key
sudo ssh-keygen -t dsa -f /etc/ssh/ssh_host_dsa_key
sudo ssh-keygen -t ecdsa -f /etc/ssh/ssh_host_ecdsa_key
sudo ssh-keygen -t ed25519 -f /etc/ssh/ssh_host_ed25519_key




至此，
vs2019 + linux 项目可以正常工作




LINUX compile command line:
cd xxlib_cpp/test_linux_1
g++ ../xxlib/ikcp.c main.cpp -O3 -std=c++17 -pthread -o main -I./ -lboost_context -I../xxlib











某种典型的运行方式：
git clone https://github.com/denghe/xxlib_cpp
cd xxlib_cpp/server_game
cp ../gens/output/cfg.bin .
../compile.sh
./main




其他：

deboost.context 常规编译

mkdir xxx
cd xxx
cmake ..
make

合并 mac ios .a:

lipo -create libfcontext_ios.a libfcontext_mac.a -output libfcontext.a







ubuntu 开启性能模式

sudo systemctl disable ondemand

after reboot: ( 应该看到 performance )

cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor







gitlab 停电导致项目 404 修复

admin 登录，浏览所有 projects, 定位到目标, 点击 超链接部分 进入明细, trigger repository check
注意查看 repocheck.log 以及 Gitaly relative path: @hashed/??/??/????????????????????????????????????????????????.git
进入默认安装路径 /var/opt/gitlab/git-data/repositories/@hashed/??/??/????????????????????????????????????????????????.git
执行 find ./objects/ -size 0 -exec rm -f {} \; 删掉 0 字节损坏文件
进入 ./refs/heads
删除出问题的 分支名字 例如 master
回到 ????????????????????????????????????????????????.git 后执行 git fsck
显示 done 修复成功
