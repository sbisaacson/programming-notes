# Running sage in docker

I'm running docker in [rootless mode][1]. This requires an environment
variable:

```sh
export DOCKER_HOST=unix://$XDG_RUNTIME_DIR/docker.sock
```

To run in a terminal:

```sh
docker run -it sagemath/sagemath:latest
```

To run a notebook server:

```sh
docker run -p8888:8888 sagemath/sagemath:latest sage-jupyter
```

To mount a directory visible inside the session:

```sh
docker run -v "$PWD":/home/sage/data -it sagemath/sagemath:latest
```

[1]: https://docs.docker.com/engine/security/rootless/
