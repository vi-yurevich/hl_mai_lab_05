#!/bin/bash

export DB_HOST=127.0.0.1
export DB_PORT=6033
export DB_LOGIN=stud
export DB_PASSWORD=stud
export DB_DATABASE=archdb
export CACHE=127.0.0.1:6379
export QUEUE_HOST=127.0.0.1:9092
export QUEUE_TOPIC=event_server
export QUEUE_GROUP_ID=0

if [[ "$1" == "-d" ]]; then
    cd docker 
    if [[ "$1" == "-pr" ]]; then
        docker container prune
        docker volume prune
        shift
    fi

    if [[ "$1" == "-bl" ]]; then
        docker-compose up --build
    else
        docker-compose up
    fi

elif [[ "$1" == "-u" ]]; then
    ./build/user_service
elif [[ "$1" == "-l" ]]; then
    ./build/lecture_service
elif [[ "$1" == "-c" ]]; then
    ./build/conference_service
elif [[ "$1" == "-w" ]]; then
    ./build/user_writer
fi