// api.js
export async function fetchWeatherData() {
    const response = await fetch('/api/weather/last24hours');
    if (!response.ok) {
      throw new Error('Netzwerkantwort war nicht in Ordnung');
    }
    return response.json();
  }
  