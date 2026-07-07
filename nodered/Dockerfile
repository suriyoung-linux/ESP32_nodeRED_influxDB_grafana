FROM nodered/node-red:latest

RUN npm install --no-fund --no-update-notifier \
    node-red-dashboard@3.6.6 \
    node-red-contrib-influxdb@0.7.0

COPY flows/esp32_level3_dashboard.json /usr/src/node-red/flows/esp32_level3_dashboard.json
COPY entrypoint.sh /usr/src/node-red/esp32-entrypoint.sh

USER root
RUN chmod +x /usr/src/node-red/esp32-entrypoint.sh
USER node-red

ENTRYPOINT ["/usr/src/node-red/esp32-entrypoint.sh"]
