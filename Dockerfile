FROM debian:bullseye-slim AS build
RUN apt update && apt install -y \
  g++ \
  cmake \
  libhidapi-dev \
  libserialport-dev \
  nlohmann-json3-dev
COPY . /inverter-tools
WORKDIR /inverter-tools
RUN cmake . && make

FROM debian:bullseye-slim
RUN apt update && apt install -y \
  libhidapi-hidraw0 \
  libserialport0 && \
  rm -rf /var/lib/apt/lists/*
COPY --from=build /inverter-tools/inverterd /usr/local/bin/inverterd
COPY --from=build /inverter-tools/inverterctl /usr/local/bin/inverterctl
ENTRYPOINT ["/usr/local/bin/inverterd"]
CMD ["--help"]
