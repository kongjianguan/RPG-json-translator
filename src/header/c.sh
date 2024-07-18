clear
if g++ tran.cpp -o ./bin/tran -lcurl -lcryptopp -ljsoncpp;then
    echo 编译成功
else 
    echo 编译失败
fi