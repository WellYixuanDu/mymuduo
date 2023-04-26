# mymuduo
mymuduo是基于Reactor模式的网络库，遵循one loop per thread的思想，核心是个事件循环EventLoop，用于响应计时器和IO事件。采用基于对象的设计风格，事件回调接口多以function + bind表达。
## 运行方式
直接运行 `./autobuild.sh`脚本即可编译好对应的动态库。
`example`文件夹下可以通过make进行测试。
## tools
tools目录下存放了一些工具类
### noncopyable类
noncopyable类通过将拷贝构造函数以及赋值构造函数delete掉来表示禁止拷贝的行为，其他类通过继承该类来禁止拷贝。
### copyable类
copyable类则表示默认行为，即支持拷贝。
### CurrentThread类
CurrentThread类利用线程的局部存储空间，即`TLS`,存储了当前线程的tid，提供了供线程使用的`int tid()`接口。
### Thread类
Thread对线程对象进行了一定的封装，提供了 `void start()`，`void join`等方法，在创建线程对象时，通过传入线程函数以及线程名字来进行该线程对象的初始化，通过调用`start()`方法来进行线程的开启执行，`start()`方法中新建一个线程并开启之前绑定的线程函数的执行，在这其中还包含线程tid值的获取等操作。
### Logger类
通过单例模式封装了一个简单的日志类。
### Timestamp类
Timestamp类封装了时间的方法。

## net
net目录下存放了网络核心库
### InetAddress类
InetAddress类封装了Ip与port。
### Channel类
Channel类封装了socket，感兴趣的Event，以及Poller返回的具体事件，保存着该channel所在的EventLoop指针，以及index和相应事件对应的回调函数。

- `setReadCallback(ReadEventCallback cb)` 等回调函数的设置方法，由于channel中保存着poller返回的具体事件即`revents`，那么将根据具体的事件触发相对应的回调函数，因此回调函数将在channel中进行保存封装。类似方法相同。
- `enableReading()`,设置fd相应的事件状态，每一个fd都拥有属于自己的事件，因此对外提供事件状态的设置接口以供调用，在事件状态修改后，会调用`update()`方法，通过保存的`EventLoop()`指针进行`Poller`中该fd的相应`events`事件的设置，并在这个过程中channel被加入到poller事件中。
- `remove()方法类似，通过`EventLoop()`指针进行`Poller`中该`channel`的移除。
- `handleEvent(Timestamp receiveTime)`根据poller通知的channel发生的具体事件，由channel负责调用具体的回调操作。

### Poller类
poller类充当事件分发器的作用，封装了poller所属的事件循环以及以键值对形式存在的Channel，提供了`poll(int timeoutMs, ChannelList *activeChannels)`，`updateChannel(Channel *channel)`, `removeChannel(Channel *channel)`的接口，通过`newDefaultPoller(EventLoop *loop)`方法来获取Poller的一个实例，默认是EPoll。

### EPollPoller类
Poller的子类，包含了epollfd_，以及相对应的events成员，其中epollfd_在poll构造函数中通过`eopll_create()`创建。

- `poll(int timeoutMs, ChannelList *activeChannels)`方法调用`epoll_wait`来等待IO事件的发生，并将到来的事件放入activeChannels中，并通知相对应的channel进行到来事件的设置。
- `updateChannel(Channel *channel)`通过`epoll_ctl()`方法来对channel进行update操作。
- `removeChannel(Channel *channel)`方法将channel从poller中删除，并进行update操作。
### EventLoop类
EventLoop类为事件循环类，主要包含channel以及Poller两大模块，一个Loop对应着一个Poller，因此，Loop中包含着一个poller_的指针；拥有一个activeChannels_，以及一些基本标志位的数据成员。

- 具有一个wakeupFd_，用于唤醒阻塞在epoll_wait的线程,该channel会被注册在poller中。
- `loop()`事件循环的开启，将循环调用poller的poll方法进行IO事件的监听，当有IO事件到来时，通知channel处理相应的IO事件，处理完成之后，执行当前EventLoop事件循环所需要处理的回调操作。
- `quit()` ，退出事件循环有两种情况，1.loop在自己的线程中调用quit  2. 在非loop的线程中，调用loop的quit，如果在自身线程中则直接退出，如果在其他线程中，则要进行唤醒，避免一直阻塞。
- `runInLoop(Functor cb)`如果在自己线程则直接执行该函数，如果在非loop线程则需要唤醒loop所在线程执行cb，runInLoop函数可能是在loop过程中被调用的。
- 封装了一些更新判断Channel的方法。
### EventLoopThread类
依照one loop per thread 的思想，EventLoopThread类为每一个线程绑定一个loop，即在线程内执行loop操作。
- `startLoop()` 将启动底层的新线程，并通过条件变量来等待新线程中loop_的创建与循环的开启，最终返回loop对象。
### EventLoopThreadPool类
EventLoopThreadPool类创建了多个EventLoopThread以及与它对应的EventLoop，除此之外它自身拥有一个baseLoop。

- `start(const ThreadInitCallback &cb)`开启所有EventLoopThread。
- `getNextLoop()`通过轮询的方式来获得下一个处理事件的loop
- `getAllLoops()`返回所有的loop
### Socket类
Socket类对sockfd进行了一些基本方法的封装，如bind,accept等。
### Buffer类
Buffer类封装了一个缓冲区，里面通过设置一个固定的头部来解决了粘包问题，同时，在从fd上读取数据时，通过开辟一个栈上空间，来读取fd上的数据，避免多次系统调用的开销。
### Acceptor类
Acceptor封装了listenfd的相关操作，本质上即为一个特殊的channel，负责监听新用户的连接，绑定的loop为用户定义的baseLoop也就是mainLoop。
### TCPConnection类
TCPConnection类负责连接的建立，与该连接上消息的收发，它具有连接建立以及读写消息等的回调，同时具备发送缓冲区以及接受缓冲区。
### TcpServer类
TcpServer类封装了用户自定义的也就是baseLoop以及EventLoopThreadPool并且对外提供了相关回调的设置方法。
- `start()`方法开启了线程池并使acceptorChannel开启监听。
- `newConnection()` 该函数注册到acceptor的连接到来回调中，当有连接到来的时候便调用这个回调，会轮询选择一个loop，并创建TcpConnectionPtr对象来管理连接。
