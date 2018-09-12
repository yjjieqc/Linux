## 共享库

共享库的创建比较简单，基本有两步，首先用-fPIC和-fpic创建目标文件，PIC或者pic表示位置无关代码，然后就可以使用一下的格式来创建共享库了： gcc -shared -Wl,-soname,your_sonmae -o library_name file_list library_list,下面时使用a.c和b.c创建库的示例：

![共享库命名]（./figures/so2.png）

### 基础班使用

gcc -fPIC -c a.c
gcc -fPIC -c b.c
gcc -shared -Wl, -o libmyab.so a.o b.o

### 就业班使用

gcc -shared -Wl,-soname,libmyab.so.1 -o libmyab.so.1.0.1 a.o b.o

