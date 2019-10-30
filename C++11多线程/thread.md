## 多线程库简介

C++98标准中并没有线程库的存在，而在C++11中终于提供了多线程的标准库，提供了管理线程、保护共享数据、线程间同步操作、原子操作等类。

多线程库对应的头文件是`#include <thread>`，类名为`std::thread`。

一个简单的串行程序如下：
```
#include <iostream>
#include <thread>

void function_1() {
    std::cout << "I'm function_1()" << std::endl;
}

int main() {
    function_1();
    return 0;
}
```

这是一个典型的单线程的单进程程序，任何程序都是一个进程，`main()`函数就是其中的主线程，单个线程都是顺序执行。

将上面的程序改造成多线程程序其实很简单，让function_1()函数在另外的线程中执行：
```
#include <iostream>
#include <thread>

void function_1() {
    std::cout << "I'm function_1()" << std::endl;
}

int main() {
    std::thread t1(function_1);
    // do other things
    t1.join();
    return 0;
}
```
分析：

1. 首先，构建一个`std::thread`对象`t1`，构造的时候传递了一个参数，这个参数是一个函数，这个函数就是这个线程的入口函数，函数执行完了，整个线程也就执行完了。

2. 线程创建成功后，就会立即启动，并没有一个类似start的函数来显式的启动线程。

3. 一旦线程开始运行， 就需要显式的决定是要等待它完成(`join`)，或者分离它让它自行运行(`detach`)。注意：只需要在`std::thread`对象被销毁之前做出这个决定。这个例子中，对象`t1`是栈上变量，在`main`函数执行结束后就会被销毁，所以需要在`main`函数结束之前做决定。

4. 这个例子中选择了使用`t1.join()`，主线程会一直阻塞着，直到子线程完成，`join()`函数的另一个任务是回收该线程中使用的资源。

线程对象和对象内部管理的线程的生命周期并不一样，如果线程执行的快，可能内部的线程已经结束了，但是线程对象还活着，也有可能线程对象已经被析构了，内部的线程还在运行。

假设`t1`线程是一个执行的很慢的线程，主线程并不想等待子线程结束就想结束整个任务，直接删掉`t1.join()`是不行的，程序会被终止（析构`t1`的时候会调用`std::terminate`，程序会打印`terminate called without an active exception`）。

与之对应，我们可以调用`t1.detach()`，从而将`t1`线程放在后台运行，**所有权和控制权被转交给C++运行时库**，以确保与线程相关联的资源在线程退出后能被正确的回收。参考UNIX的**守护进程(daemon process)**的概念，这种被分离的线程被称为**守护线程(daemon threads)**。线程被分离之后，即使该线程对象被析构了，线程还是能够在后台运行，只是由于对象被析构了，主线程不能够通过对象名与这个线程进行通信。例如：
```
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

// 使用 t1.detach()时
// test() finished
// I'm function_1()

// 使用 t1.join()时
// I'm function_1()
// test() finished
```

分析：

1.由于线程入口函数内部有个500ms的延时，所以在还没有打印的时候，test()已经执行完成了，t1已经被析构了，但是它负责的那个线程还是能够运行，这就是detach()的作用。

2.如果去掉main函数中的1s延时，会发现什么都没有打印，因为主线程执行的太快，整个程序已经结束了，那个后台线程被C++运行时库回收了。

3.如果将t1.detach()换成t1.join()，test函数会在t1线程执行结束之后，才会执行结束。

一旦一个线程被分离了，就不能够再被join了。如果非要调用，程序就会崩溃，可以使用joinable()函数判断一个线程对象能否调用join()。
```
void test() {
    std::thread t1(function_1);
    t1.detach();

    if(t1.joinable())
        t1.join();

    assert(!t1.joinable());
}
```