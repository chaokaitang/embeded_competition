import * as echarts from '../../ec-canvas/echarts';

const app = getApp();

function initChart(canvas, width, height, dpr) {
  const chart = echarts.init(canvas, null, {
    width: width,
    height: height,
    devicePixelRatio: dpr // new
  });
  canvas.setChart(chart);

  var option = {
    title: {
      text: '商品吸引力',
      left: 'center'
    },
    legend: {
      data: ['乒乓球', '胶带', 'U盘','STM32'],
      top: 50,
      left: 'center',
      backgroundColor: 'red',
      z: 100
    },
    grid: {
      containLabel: true
    },
    tooltip: {
      show: true,
      trigger: 'axis'
    },
    xAxis: {
      type: 'category',
      boundaryGap: false,
      data: ['周一', '周二', '周三', '周四', '周五', '周六', '周日'],
      // show: false
    },
    yAxis: {
      x: 'center',
      type: 'value',
      splitLine: {
        lineStyle: {
          type: 'dashed'
        }
      }
      // show: false
    },
    series: [{
      name: '乒乓球',
      type: 'line',
      smooth: true,
      data: [40, 36, 65, 30, 78, 40, 36]
    }, {
      name: '胶带',
      type: 'line',
      smooth: true,
      data: [12, 50, 51, 35, 70, 30, 20]
    }, {
      name: 'U盘',
      type: 'line',
      smooth: true,
      data: [10, 30, 31, 50, 40, 20, 10]
    }, {
      name: 'STM32',
      type: 'line',
      smooth: true,
      data: [20, 35, 11, 53, 20, 70, 77]
    }]
  };

  chart.setOption(option);
  return chart;
}

Page({
  
  onLoad: function (options) {
    if (options.data) {
      const productData = JSON.parse(decodeURIComponent(options.data));
      this.initChart(productData);
    }
  },
  
  data: {
    ec: {
      onInit: initChart
    }
  },

  onReady() {
  }
});
