deboost.context 常规编译

mkdir xxx
cd xxx
cmake ..
make

合并 mac ios .a:

lipo -create libfcontext_ios.a libfcontext_mac.a -output libfcontext.a