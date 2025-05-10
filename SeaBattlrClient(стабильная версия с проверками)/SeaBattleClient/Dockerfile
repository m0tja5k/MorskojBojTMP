# Устанавливаем базовый образ
FROM ubuntu:latest

ENV TZ=Europe/Moscow
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

# Обновление и установка зависимостей
RUN apt-get update
RUN apt-get install -y qt6-base-dev
RUN apt-get install -y libgl1-mesa-dev
RUN apt-get install -y openssl
RUN apt-get install -y build-essential
RUN apt-get install -y qtchooser 
RUN apt-get install -y qt6-base-dev-tools 
RUN apt-get install -y qmake6

# Копируем файлы проекта в контейнер
WORKDIR /server
COPY *.cpp /server
COPY *.h /server
COPY *.pro  /server
COPY *.db /server
COPY *.ui /server

# Сборка проекта
RUN qmake6 SeaBattleClient.pro
RUN make

# Запускаем
#WORKDIR /test # Устанавливаем WORKDIR перед запуском
ENTRYPOINT ["./SeaBattleClient"] 