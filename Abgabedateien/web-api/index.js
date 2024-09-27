const express = require('express');
const mysql = require('mysql2/promise');
const app = express();
const PORT = process.env.PORT || 3000;

// MySQL-Datenbankverbindung konfigurieren
const dbConfig = {
  host: process.env.DB_HOST || 'localhost',
  user: process.env.DB_USER || 'root',
  password: process.env.DB_PASSWORD || '',
  database: process.env.DB_NAME || 'weatherdb',
};

// Berechne die Zeit für die letzten 24 Stunden
const calculateTimeParameters = () => {
  const now = new Date((new Date()).getTime() + 60 * 60 * 1000);
  const past24Hours = new Date(now.getTime() - 24 * 60 * 60 * 1000);

  return {
    nowYear: now.getUTCFullYear(),
    nowMonth: now.getUTCMonth() + 1,  // Monat in JavaScript ist 0-basiert
    nowDay: now.getUTCDate(),
    nowHour: now.getUTCHours(),
    nowMinute: now.getUTCMinutes(),
    pastYear: past24Hours.getUTCFullYear(),
    pastMonth: past24Hours.getUTCMonth() + 1,
    pastDay: past24Hours.getUTCDate(),
    pastHour: past24Hours.getUTCHours(),
    pastMinute: past24Hours.getUTCMinutes()
  };
};

// API-Routen
app.get('/api/weather/last24hours', async (req, res) => {
  try {
    const {
      nowYear, nowMonth, nowDay, nowHour, nowMinute,
      pastYear, pastMonth, pastDay, pastHour, pastMinute
    } = await calculateTimeParameters();

    // Verbindung zur Datenbank herstellen
    const connection = await mysql.createConnection(dbConfig);

    console.log('Current Time:');
    console.log('Year:', nowYear);
    console.log('Month:', nowMonth);
    console.log('Day:', nowDay);
    console.log('Hour:', nowHour);
    console.log('Minute:', nowMinute);

    console.log('Past 24 Hours Time:');
    console.log('Year:', pastYear);
    console.log('Month:', pastMonth);
    console.log('Day:', pastDay);
    console.log('Hour:', pastHour);
    console.log('Minute:', pastMinute);
    
    // SQL-Abfrage für die letzten 24 Stunden basierend auf year, month, day, hour, minute
    const query = `
      SELECT year, month, day, hour, minute, temperature, humidity, pressure, rain 
      FROM weather_data
      WHERE (year = ? AND month = ? AND day = ? AND (hour > ? OR (hour = ? AND minute >= ?)))
      OR (year = ? AND month = ? AND day = ? AND (hour < ? OR (hour = ? AND minute <= ?)))
      OR (year = ? AND month = ? AND day > ?)
      OR (year = ? AND month > ?)
      OR (year > ?)
    `;

    const [rows] = await connection.execute(query, [
      pastYear, pastMonth, pastDay, pastHour, pastHour, pastMinute, // Bedingung für den Startzeitpunkt
      nowYear, nowMonth, nowDay, nowHour, nowHour, nowMinute,       // Bedingung für den Endzeitpunkt
      pastYear, pastMonth, pastDay,                        // Zusätzliche Bedingung für die Tage
      pastYear, pastMonth,                                // Für den Monat und das Jahr
      pastYear                                             // Für die Jahre
    ]);
    
    // Verbindung schließen
    await connection.end();
    
    // Daten als JSON zurücksenden
    res.json(rows);
  } catch (error) {
    console.error('Error fetching weather data:', error);
    res.status(500).json({ error: 'Failed to fetch weather data' });
  }
});

// Server starten
app.listen(PORT, () => {
  console.log(`Server is running on port ${PORT}`);
});
