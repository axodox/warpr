const { env } = require('process');

const target = env.ASPNETCORE_HTTPS_PORT ? `https://0.0.0.0:${env.ASPNETCORE_HTTPS_PORT}` :
  env.ASPNETCORE_URLS ? env.ASPNETCORE_URLS.split(';')[0] : 'https://0.0.0.0:7074';

const PROXY_CONFIG = [
  {
    context: [
      "/api",
    ],
    target,
    ws: true,
    secure: false
  }
]

module.exports = PROXY_CONFIG;
