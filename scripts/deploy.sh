echo $CI_SERVER_PASSWORD
echo "$CI_SERVER_PASSWORD"
echo "$CI_SERVER_PASSWORD" | sudo -S -i
docker pull $CI_DOCKER_IMAGE_NAME
docker stop $CI_DOCKER_CONTAINER_NAME
docker rm $CI_DOCKER_CONTAINER_NAME
docker run -itdp $CI_PROGRAM_PORT_HOST:$CI_PROGRAM_PORT_LOCAL --restart=always --name $CI_DOCKER_CONTAINER_NAME $CI_DOCKER_IMAGE_NAME