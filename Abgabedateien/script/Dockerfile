# Verwende das offizielle Python 3.11.2 Slim-Image
FROM python:3.11.2-slim

# Arbeitsverzeichnis im Container festlegen
WORKDIR /app

# Kopiere die Anwendungsdateien ins Arbeitsverzeichnis
COPY script/requirements.txt .
COPY script/weather_data_ingest.py .

# Installiere alle Abhängigkeiten
RUN pip install --no-cache-dir -r requirements.txt

# Das Kommando, um das Skript zu starten
CMD ["python", "-u", "weather_data_ingest.py"]
