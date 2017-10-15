Make sure official image `gcc` is available

```
docker run --name websocket_server -dit -v /Users/siruitan/try_websocket/server/:/server gcc
docker run --name websocket_client_1 -dit -v /Users/siruitan/try_websocket/client/:/client gcc
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
bin/client
```
