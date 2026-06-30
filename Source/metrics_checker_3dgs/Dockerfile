FROM ubuntu:24.04

WORKDIR /app

RUN apt-get -qq update && apt-get install -y python3 python3-pip python3.12-venv

RUN python3 -m venv /app/venv

ENV PATH="/app/venv/bin:$PATH"