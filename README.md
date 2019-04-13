Linux 推荐 ubuntu 16 18+
apt-get -y install g++ uuid-dev
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