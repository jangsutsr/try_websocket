### Introduction
Inspired by [memcached source](https://github.com/memcached/memcached), this project tried to implement [websocket protocol](https://tools.ietf.org/html/rfc6455) using [libevent](http://www.wangafu.net/~nickm/libevent-book/) and [pthread](http://man7.org/linux/man-pages/man7/pthreads.7.html).

### Set up
Make sure official image `gcc` is available

```
docker run --name websocket_server -dit -v /Users/siruitan/try_websocket/server/:/server gcc_with_gdb:1.0
docker run --name websocket_client_1 -dit -v /Users/siruitan/try_websocket/client/:/client gcc_with_gdb:1.0
```

```
docker exec -it websocket_server bash
# Inside container
cd server/
make clean
make
bin/server
```

```
docker exec -it websocket_client_1 bash
# Inside container
cd client/
make clean
make
# Use `docker inspect websocket_server | grep IP` to find server IP
bin/client --address <IP of server> --port 2333
```
