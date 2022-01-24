#!/bin/sh

set -e

AGENT_DIR=${AGENT_DIR:=/data/carota}

sudo mkdir -m 755 -p ${AGENT_DIR}
sudo chmod 777 ${AGENT_DIR}