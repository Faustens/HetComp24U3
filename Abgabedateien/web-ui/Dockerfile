# Verwende Nginx-Image
FROM nginx:alpine

# Kopiere den fertigen Build in das Nginx-Verzeichnis
COPY dist/ /usr/share/nginx/html

# Kopiere die Nginx-Konfigurationsdatei in den Container
COPY nginx.conf /etc/nginx/conf.d/default.conf

# Exponiere Port 80
EXPOSE 80

# Start Nginx
CMD ["nginx", "-g", "daemon off;"]
