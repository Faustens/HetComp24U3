# Verwende ein Node.js-Image
FROM node:18-alpine

# Arbeitsverzeichnis erstellen
WORKDIR /app

# Package.json und package-lock.json kopieren
COPY package*.json ./

# Abhängigkeiten installieren
RUN npm install

# Den Rest der App kopieren
COPY . .

# Der Port, den die API nutzt
EXPOSE 3000

# Start-Befehl für die API
CMD ["node", "index.js"]
