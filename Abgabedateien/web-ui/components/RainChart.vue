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
  name: 'RainChart',
  props: ['data', 'title'],
  mounted() {
    this.createChart();
  },
  methods: {
    createChart() {
      new Chart(this.$refs.canvas, {
        type: 'bar',
        data: this.data,
        options: {
          responsive: true,
          maintainAspectRatio: false,
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
              beginAtZero: true,
              min: 0,
              max: 1,
              ticks: {
                stepSize: 1,
                callback: function(value) {
                  return value === 1 ? 'Regen' : 'Kein Regen';
                }
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
