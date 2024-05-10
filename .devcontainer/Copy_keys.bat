docker exec -it devcontainer_netlib ls
docker exec -it devcontainer_netlib rm -rf /root/.ssh/
docker cp %USERPROFILE%/.ssh/ devcontainer_netlib:/root/.ssh/
docker exec -it devcontainer_netlib chmod 400 -R /root/.ssh/
PAUSE