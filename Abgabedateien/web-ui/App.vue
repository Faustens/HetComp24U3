<template>
  <div id="app">
    <h1>Wetterdaten</h1>
    <LineChart v-if="temperatureData" :data="temperatureData" title="Temperatur" type="temperature"/>
    <LineChart v-if="pressureData" :data="pressureData" title="Luftdruck" type="pressure"/>
    <LineChart v-if="humidityData" :data="humidityData" title="Luftfeuchtigkeit" type="humidity"/>
    <RainChart v-if="rainData" :data="rainData" title="Regenfall"/>
  </div>
</template>

<script>
import LineChart from './components/LineChart.vue';
import RainChart from './components/RainChart.vue';
import { fetchWeatherData } from './api';

export default {
  name: 'App',
  components: { LineChart, RainChart },
  data() {
    return {
      temperatureData: null,
      pressureData: null,
      humidityData: null,
      rainData: null,
    };
  },
  async mounted() {
    const refreshInterval = 1 * 60 * 1000; // 5 Minuten
    this.refreshTimer = setInterval(() => {
      window.location.reload();
    }, refreshInterval);
    
    try {
      const allData = await fetchWeatherData();
      this.processData(allData); // Aufruf der Methode zur Datenverarbeitung
    } catch (error) {
      console.error('Fehler beim Abrufen der Wetterdaten:', error);
    }
  },
  methods: {
    processData(data) {
      const todayStart = new Date();
      todayStart.setHours(0, 0, 0, 0);

      const todayEnd = new Date();
      todayEnd.setHours(23, 50, 0, 0);

      const filteredData = data.filter(item => {
        const date = new Date(item.year, item.month - 1, item.day, item.hour, item.minute);
        return date >= todayStart && date <= todayEnd;
      });

      const formatData = (key) => ({
        labels: filteredData.map(item => new Date(item.year, item.month - 1, item.day, item.hour, item.minute)),
        datasets: [{
          label: key,
          data: filteredData.map(item => parseFloat(item[key]).toFixed(2)),
          borderColor: 'rgba(75, 192, 192, 1)',
          backgroundColor: 'rgba(75, 192, 192, 0.2)',
          fill: false,
        }]
      });

      const formatRainData = () => ({
        labels: filteredData.map(item => new Date(item.year, item.month - 1, item.day, item.hour, item.minute)),
        datasets: [{
          label: 'Regen',
          data: filteredData.map(item => item.rain ? 1 : 0),
          backgroundColor: 'rgba(255, 99, 132, 0.2)',
          borderColor: 'rgba(255, 99, 132, 1)',
        }]
      });

      this.temperatureData = formatData('temperature');
      this.pressureData = formatData('pressure');
      this.humidityData = formatData('humidity');
      this.rainData = formatRainData();
    }
  }
};
</script>

<style>
#app {
  font-family: Avenir, Helvetica, Arial, sans-serif;
  text-align: center;
  color: #2c3e50;
  margin-top: 60px;
}
</style>
