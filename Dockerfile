FROM ubuntu:latest

WORKDIR /app

ENV TZ=Europe/Moscow
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

RUN apt-get update
RUN apt-get install -y qt6-base-dev build-essential qtchooser qt6-base-dev-tools qmake6

COPY . /app/

# Проверка:
RUN ls -l /app
RUN qmake6 /app/echoServer.pro && ls -l /app #Добавляем проверку

RUN make -C /app

ENTRYPOINT ["/app/echoServer"] 