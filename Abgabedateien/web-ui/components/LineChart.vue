<template>
  <div>
    <h2>{{ title }}</h2>
    <canvas ref="canvas"></canvas>
  </div>
</template>

<script>
import { Chart, registerables } from 'chart.js';
import 'chartjs-adapter-date-fns'; // Adapter für Zeitangaben in Chart.js

Chart.register(...registerables);

export default {
  name: 'LineChart',
  props: ['data', 'title', 'type'],
  mounted() {
    this.createChart();
  },
  methods: {
    createChart() {
      const yAxisLimits = {
        temperature: { min: -20, max: 50, stepSize: 5, maxTicksLimit: 10, unit: '°C'},
        pressure: { min: 950, max: 1100, stepSize: 10, maxTicksLimit: 10, unit: ' hPa' },
        humidity: { min: 0, max: 100, stepSize: 5, maxTicksLimit: 20, unit: '%' },
      };

      new Chart(this.$refs.canvas, {
        type: 'line',
        data: this.data,
        options: {
          responsive: false,
          maintainAspectRatio: true,
          scales: {
            x: {
              type: 'time',
              time: {
                unit: 'minute',
                stepSize: 10,
                displayFormats: {
                  minute: 'HH:mm'
                },
                min: new Date(new Date().setHours(0, 0, 0, 0)),
                max: new Date(new Date().setHours(23, 50, 0, 0))
              },
              ticks: {
                source: 'auto',
                stepSize: 10,
                autoSkip: false
              },
            },
            y: {
              customProperties: {
                yAxisLimits: yAxisLimits[this.type],
              },
              beginAtZero: false,
              min: yAxisLimits[this.type].min,
              max: yAxisLimits[this.type].max,
              stepSize: yAxisLimits[this.type].stepSize,
              ticks: {
                callback: function(value) {
                  return value + '' + yAxisLimits[this.type].unit;
                }.bind(this)
              }
            }
          }
        }
      });
    }
  }
};
</script>

<style scoped>
canvas {
  width: 100% !important;
  height: 400px !important;
}
</style>
