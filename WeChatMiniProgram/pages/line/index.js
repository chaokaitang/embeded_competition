import * as echarts from '../../ec-canvas/echarts';
const moment = require('../../utils/moment.min.js'); // 导入 moment.js 库

const app = getApp();

var chart = null;

function initChart(canvas,width,height,dpr) {
    chart = echarts.init(canvas, null, {
    width: width,
    height: height,
    devicePixelRatio: dpr // new
  });
  canvas.setChart(chart);
  return chart;
}

Page({
  data: {
    ec: {
      onInit: initChart
    },
    day: '', // 初始化 day 变量
    productA: 0,
    productB: 0,
    productC: 0,
    productD: 0,
    dayOfWeek: 0,

  },

  onReady() {

  },

  onLoad: function (options) {
    
    if (options.data) {
      const productData = JSON.parse(decodeURIComponent(options.data));
      // this.initChart(productData);
      this.gettoken(); // 获取 token
      this.getshadow(); // 获取设备影子数据
    }
  },

  // 获取 token 方法
  gettoken: function () {
    console.log("开始获取token...");
    var that = this;
    wx.request({
      url: 'https://iam.cn-north-4.myhuaweicloud.com/v3/auth/tokens',
      data: '{"auth": { "identity": {"methods": ["password"],"password": {"user": {"name": "IAM_admin","password": "LiuBingYan0808","domain": {"name": "hw001835075"}}}},"scope": {"project": {"name": "cn-north-4"}}}}',
      method: 'POST',
      header: { 'content-type': 'application/json' },
      success: function (res) {
        console.log("获取token成功");
        var token = res.header['X-Subject-Token'];
        console.log("token=" + token);
        wx.setStorageSync('token', token); // 将 token 存储到缓存中
      },
      fail: function () {
        console.log("获取token失败");
      }
    });
  },

  // 获取设备影子数据方法
  getshadow: function () {
    console.log("开始获取影子");// 打印消息，表示开始获取设备影子数据
    var that = this;  // 在回调函数中使用 that 可以访问当前页面的数据和方法
    var token = wx.getStorageSync('token');// 从缓存中读取保存的 token
    wx.request({
      url: 'https://iotda.cn-north-4.myhuaweicloud.com/v5/iot/147ce18cf445499a88dbb1a95436cea0/devices/664f6c3cc2c0cb155cbe0cd3_Hispark/shadow',
      data: '',
      method: 'GET',
      header: {
        'content-type': 'application/json',
        'X-Auth-Token': token
      },
      success: function (res) {
        var shadow = JSON.stringify(res.data.shadow[0].reported.properties);
        console.log('设备影子数据：' + shadow);

        var productA = parseInt(res.data.shadow[0].reported.properties.left_up);
        var productB = parseInt(res.data.shadow[0].reported.properties.right_up);
        var productC = parseInt(res.data.shadow[0].reported.properties.left_down);
        var productD = parseInt(res.data.shadow[0].reported.properties.right_down);
        console.log('productA=' + productA);
        console.log('productB=' + productB);
        console.log('productC=' + productC);
        console.log('productD=' + productD);

        that.setData({ 
          productA: productA,
          productB: productB,
          productC: productC,
          productD: productD 
        }, function() {
          // Ensure data is updated before moving forward
          that.updateChartData();
        });
      },
      fail: function () {
        console.log("获取影子失败");// 打印消息，表示获取设备影子数据失败
      },
      complete: function () {
        console.log("获取影子完成");// 打印消息，表示获取设备影子数据完成
      }
    });
  },

  // Update chart data based on the current day
  updateChartData: function() {
    var dayOfWeek = moment().weekday(); // 0是星期天，1是星期一，以此类推
    console.log('今天是星期：' + dayOfWeek);

    var dataA = [40, 36, 65, 30, 78, 40, 36];
    var dataB = [12, 50, 51, 35, 70, 30, 20];
    var dataC = [10, 30, 31, 50, 40, 20, 10];
    var dataD = [20, 35, 11, 53, 20, 70, 77];

    switch(dayOfWeek) {
      case 1: // 星期一
        dataA[0] = this.data.productA;
        dataB[0] = this.data.productB;
        dataC[0] = this.data.productC;
        dataD[0] = this.data.productD;
        break;
      case 2: // 星期二
        dataA[1] = this.data.productA;
        dataB[1] = this.data.productB;
        dataC[1] = this.data.productC;
        dataD[1] = this.data.productD;
        break;
      case 3: // 星期三
        dataA[2] = this.data.productA;
        dataB[2] = this.data.productB;
        dataC[2] = this.data.productC;
        dataD[2] = this.data.productD;
        break;
      case 4: // 星期四
        dataA[3] = this.data.productA;
        dataB[3] = this.data.productB;
        dataC[3] = this.data.productC;
        dataD[3] = this.data.productD;
        break;
      case 5: // 星期五
        dataA[4] = this.data.productA;
        dataB[4] = this.data.productB;
        dataC[4] = this.data.productC;
        dataD[4] = this.data.productD;
        break;
      case 6: // 星期六
        dataA[5] = this.data.productA;
        dataB[5] = this.data.productB;
        dataC[5] = this.data.productC;
        dataD[5] = this.data.productD;
        break;
      case 0: // 星期天
        dataA[6] = this.data.productA;
        dataB[6] = this.data.productB;
        dataC[6] = this.data.productC;
        dataD[6] = this.data.productD;
        break;
    }

    this.setData({
      day: dayOfWeek // 将星期几存入 data 中的 day 变量
    });

    // 构建 seriesData
    var seriesData = [
      {
        name: '兵乓球',
        type: 'line',
        smooth: true,
        data: dataA
      },
      {
        name: '胶带',
        type: 'line',
        smooth: true,
        data: dataB
      },
      {
        name: 'U盘',
        type: 'line',
        smooth: true,
        data: dataC
      },
      {
        name: 'STM32',
        type: 'line',
        smooth: true,
        data: dataD
      }
    ];
    console.log('Updated data for the chart:'+ seriesData);
    chart.setOption(this.getoption(seriesData), true);
  },

getoption:function(seriesData) {
    // 图表内容配置项
    const option = {
      title: {
        text: '商品吸引力',
        left: 'center'
      },
      legend: {
        data: ['兵乓球', '胶带', 'U盘', 'STM32'],
        top: 50,
        left: 'center',
        // backgroundColor: 'red',
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
      },
      yAxis: {
        x: 'center',
        type: 'value',
        splitLine: {
          lineStyle: {
            type: 'dashed'
          }
        }
      },
      
      series: seriesData
    };
     return option;
    }

});
