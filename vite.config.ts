import { defineConfig } from 'vite'
import react from '@vitejs/plugin-react'

export default defineConfig({
  plugins: [react()],
  server: {
    proxy: {
      '/auth': {
        target: 'http://192.168.35.124:9000',
        changeOrigin: true,
        configure: (proxy, options) => {
          proxy.on('proxyReq', (proxyReq, req, res) => {
            if (req.headers.authorization) {
              proxyReq.setHeader('authorization', req.headers.authorization);
            }
          });
        }
      },
      '/video': {
        target: 'http://192.168.35.124:9000',
        changeOrigin: true,
        configure: (proxy, options) => {
          proxy.on('proxyReq', (proxyReq, req, res) => {
            if (req.headers.authorization) {
              proxyReq.setHeader('authorization', req.headers.authorization);
            }
            if (req.headers['x-video-title']) {
              proxyReq.setHeader('x-video-title', req.headers['x-video-title']);
            }
          });
        }
      },
    },
  },
})
