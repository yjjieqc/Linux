#include <iostream>
#include <thread>

void function_1() {
    //延时500ms 为了保证test()运行结束之后才打印
    std::this_thread::sleep_for(std::chrono::milliseconds(500)); 
    std::cout << "I'm function_1()" << std::endl;
}

void test() {
    std::thread t1(function_1);
    t1.detach();
    // t1.join();
    std::cout << "test() finished" << std::endl;
}

int main() {
    test();
    //让主线程晚于子线程结束
    std::this_thread::sleep_for(std::chrono::milliseconds(1000)); //延时1s
    return 0;
}