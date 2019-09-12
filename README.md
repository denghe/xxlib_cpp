系统：
ubuntu 18.04 LTS server，安装时勾安 OpenSSH server 默认安全选项

安装编译器，依赖库：
sudo apt install gcc-8 g++-8 gdb gdbserver libboost-context-dev libuv1-dev uuid-dev libsqlite3-dev libmariadb-dev

设置 gcc g++ 命令指向 8.0:
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-8 60 --slave /usr/bin/g++ g++ /usr/bin/g++-8


至此，
vs2019 + linux 项目可以正常工作



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