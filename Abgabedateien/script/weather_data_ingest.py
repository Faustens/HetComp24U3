import json
import mysql.connector
import paho.mqtt.client as mqtt

# MQTT Broker-Details
BROKER_ADDRESS = "mqtt5"  # Name des Mosquitto-Containers im Docker-Netzwerk
BROKER_PORT = 1883
MQTT_USER = "default_user"
MQTT_PW = "DefaultPassword"
TOPIC = "weather"

# MySQL-Datenbank-Details
DB_HOST = "weather_db"  # Name des MySQL-Containers im Docker-Netzwerk
DB_USER = "weather_user"
DB_PASSWORD = "WeatherPassword"
DB_NAME = "weather_db"
DB_PORT = 3306

# Verbindung zur MySQL-Datenbank herstellen
def connect_to_db():
    return mysql.connector.connect(
        host=DB_HOST,
        user=DB_USER,
        password=DB_PASSWORD,
        database=DB_NAME,
	port=DB_PORT
    )

# Callback, wenn eine Nachricht empfangen wird
def on_message(client, userdata, msg):
    try:
        # Die empfangene Nachricht wird als JSON interpretiert
        data = json.loads(msg.payload.decode("utf-8"))

        # Verbindung zur MySQL-Datenbank
        conn = connect_to_db()
        cursor = conn.cursor()

        # SQL-Statement zum Einfügen der Daten
        insert_query = """
        INSERT INTO weather_data (year, month, day, hour, minute, temperature, pressure, humidity, rain)
        VALUES (%s, %s, %s, %s, %s, %s, %s, %s, %s)
        """

        # Extrahierte Werte aus dem JSON
        values = (
            data['year'],
            data['month'],
            data['day'],
	    data['hour'],
	    data['minute'],
	    data['temperature'],
            data['pressure'],
            data['humidity'],
            data['rain']
        )

        # Ausführung des SQL-Statements
        cursor.execute(insert_query, values)
        conn.commit()

        # Verbindung schließen
        cursor.close()
        conn.close()

        print(f"Data inserted: {values}")

    except Exception as e:
        print(f"Failed to insert data: {e}")

# MQTT-Client einrichten und verbinden
def main():
    print("Skript started");
    client = mqtt.Client()
    client.username_pw_set(MQTT_USER, MQTT_PW)
    client.on_message = on_message

    client.connect(BROKER_ADDRESS, BROKER_PORT)
    client.subscribe(TOPIC, qos=2)

    client.loop_forever()

if __name__ == "__main__":
    main()
